#include <pebble.h>
#include "main.h"
#include "weekday.h"
//#include "effect_layer.h"
#include <pebble-fctx/fctx.h>
#include <pebble-fctx/fpath.h>
#include <pebble-fctx/ffont.h>

//Static and initial vars
static GFont
  FontDayOfTheWeekShortName, FontBTQTIcons;
FFont* time_font;

char  citistring[24];

static Window * s_window;

static Layer * s_canvas_background;
static Layer * s_canvas_weather_section;
static Layer * s_canvas_bt_icon;
static Layer * s_canvas_qt_icon;
Layer * time_area_layer;

static int s_hours, s_minutes, s_weekday, s_day, s_month;

//////Init Configuration///
//Init Clay
ClaySettings settings;
// Initialize the default settings
static void prv_default_settings(){
  settings.HourColorN = GColorBlack;
  settings.MinColorN = GColorBlack;
  settings.DateColorN = GColorBlack;
  settings.FrameColor2N = GColorWhite;
  settings.FrameColorN = GColorWhite;
  settings.Text1ColorN = GColorBlack;
  settings.Text2ColorN = GColorBlack;
  settings.Text3ColorN = GColorBlack;
  settings.Text4ColorN = GColorBlack;
  settings.Text5ColorN = GColorBlack;
  settings.Text6ColorN = GColorBlack;
  settings.Text7ColorN = GColorBlack;
  settings.Text8ColorN = GColorBlack;
  settings.Text9ColorN = GColorBlack;
  settings.Text10ColorN = GColorBlack;
  settings.FrameColor2 = GColorBlack;
  settings.FrameColor = GColorBlack;
  settings.Text1Color = GColorWhite;
  settings.Text2Color = GColorWhite;
  settings.Text3Color = GColorWhite;
  settings.Text4Color = GColorWhite;
  settings.Text5Color = GColorWhite;
  settings.Text6Color = GColorWhite;
  settings.Text7Color = GColorWhite;
  settings.Text8Color = GColorWhite;
  settings.Text9Color = GColorWhite;
  settings.Text10Color = GColorWhite;
  settings.HourColor = GColorWhite;
  settings.MinColor = GColorWhite;
  settings.DateColor = GColorWhite;
  settings.NightTheme = false;
  settings.HealthOff = true;
  settings.VibeOn = false;
  settings.AddZero12h = false;
  settings.RemoveZero24h = false;
  }

bool BTOn=true;
bool GPSOn=true;
bool IsNightNow=false;

static GColor ColorSelect(GColor ColorDay, GColor ColorNight){
  if (settings.NightTheme && IsNightNow && GPSOn){
    return ColorNight;
  }
  else{
    return ColorDay;
  }
}

///BT Connection
static void bluetooth_callback(bool connected){
  BTOn = connected;
}

static void bluetooth_vibe_icon (bool connected) {

  layer_set_hidden(s_canvas_bt_icon, connected);

  if(!connected && !quiet_time_is_active()) {
    // Issue a vibrating alert
    vibes_double_pulse();
  }
  else{
  if(!connected && quiet_time_is_active() && settings.VibeOn) {
      // Issue a vibrating alert
      vibes_double_pulse();
    }
  }
}

///Show sound off icon if Quiet time is active
static void quiet_time_icon () {
  if(!quiet_time_is_active()) {
  layer_set_hidden(s_canvas_qt_icon,true);
  }
}

//Add in HEALTH to the watchface

static char s_current_steps_buffer[8];
static int s_step_count = 0;

// Is step data available?
bool step_data_is_available(){
    return HealthServiceAccessibilityMaskAvailable &
      health_service_metric_accessible(HealthMetricStepCount,
        time_start_of_today(), time(NULL));
      }

// Todays current step count
static void get_step_count() {
//  if (!settings.HealthOff) {
    s_step_count = (int)health_service_sum_today(HealthMetricStepCount);/* code */
//  }
}

static void display_step_count() {
  int thousands = s_step_count / 1000;
  int hundreds = (s_step_count % 1000)/100;
  int hundreds2 = (s_step_count % 1000);

  snprintf(s_current_steps_buffer, sizeof(s_current_steps_buffer),
   "%s", "");

  if(thousands > 9) {
      snprintf(s_current_steps_buffer, sizeof(s_current_steps_buffer),
      "%d.%d%s", thousands, hundreds, "k");
  //      "%d,%d", thousands, hundreds2);
  }
  else if(thousands > 0) {
      snprintf(s_current_steps_buffer, sizeof(s_current_steps_buffer),
  //   "%d.%d%s", thousands, hundreds, "k");
        "%d,%03d", thousands, hundreds2);
  }
  else {
    snprintf(s_current_steps_buffer, sizeof(s_current_steps_buffer),
      "%d", hundreds2);
  }

}


static void health_handler(HealthEventType event, void *context) {
    if(event != HealthEventSleepUpdate) {
      get_step_count();
      display_step_count();
    }
  }


void layer_update_proc_background (Layer * back_layer, GContext * ctx){

  GRect bounds = layer_get_bounds(back_layer);
  GRect ComplicationsBand =
     PBL_IF_ROUND_ELSE(
       GRect(0, 0, bounds.size.w, bounds.size.h),
       GRect(0, 0, bounds.size.w, bounds.size.h));

  GRect TimeBand =
    PBL_IF_ROUND_ELSE(
      GRect(0, 0, bounds.size.w, 144),
      GRect(0, 0, bounds.size.w, 144));
      graphics_context_set_fill_color(ctx, ColorSelect(settings.FrameColor, settings.FrameColorN));
      graphics_fill_rect(ctx, ComplicationsBand,0,GCornersAll);
      graphics_context_set_fill_color(ctx, ColorSelect(settings.FrameColor2, settings.FrameColor2N));
      graphics_fill_rect(ctx, TimeBand,0,GCornersAll);
  }


void update_time_area_layer(Layer *l, GContext* ctx) {
  // check layer bounds
  #ifdef PBL_ROUND
     GRect bounds = layer_get_unobstructed_bounds(l);
     bounds = GRect(0, 0,bounds.size.w, bounds.size.h);
  #else
     GRect bounds = GRect (0,0,144,80);
     bounds = GRect(0,0,bounds.size.w,bounds.size.h);
  #endif

  FContext fctx;

  fctx_init_context(&fctx, ctx);
  fctx_set_color_bias(&fctx, 0);
  fctx_set_fill_color(&fctx, ColorSelect(settings.HourColor, settings.HourColorN));
 #ifdef PBL_ROUND
    //int font_size = bounds.size.h * 0.55;
    int font_size = 172;
   #elif PBL_PLATFORM_APLITE
    //int font_size = bounds.size.h * 0.65;
    int font_size = 172;
   #else
    int font_size = 172;
    #endif
    int h_adjust = 0;
    int v_adjust = 0;

      #ifdef PBL_COLOR
        fctx_enable_aa(true);
      #endif

    // if it's a round watch, EVERYTHING CHANGES
    #ifdef PBL_ROUND
      v_adjust = 0;

    #else
      h_adjust = 0;
    #endif

    FPoint time_pos;
    fctx_begin_fill(&fctx);
    fctx_set_text_em_height(&fctx, time_font, font_size);
    fctx_set_color_bias(&fctx,0);

    time_t temp = time(NULL);
    struct tm *time_now = localtime(&temp);

    char hourdraw[8];
    if(clock_is_24h_style() && settings.RemoveZero24h){
        strftime(hourdraw, sizeof(hourdraw),"%k",time_now);
    } else if (clock_is_24h_style() && !settings.RemoveZero24h) {
        strftime(hourdraw, sizeof(hourdraw),"%H",time_now);
    } else if (settings.AddZero12h) {
      strftime(hourdraw, sizeof(hourdraw),"%I",time_now);
    } else {
      strftime(hourdraw, sizeof(hourdraw),"%l",time_now);
    }

    int mindraw;
    mindraw = s_minutes;
    char minnow[3];
    snprintf(minnow, sizeof(minnow), "%02d", mindraw);

    char timedraw[6];
    snprintf(timedraw, sizeof(timedraw), "%s:%s",hourdraw,minnow);

    time_pos.x = INT_TO_FIXED(PBL_IF_ROUND_ELSE(90, 72) + h_adjust);
    time_pos.y = INT_TO_FIXED(PBL_IF_ROUND_ELSE(6, 6)  + v_adjust);

    fctx_set_offset(&fctx, time_pos);
    fctx_draw_string(&fctx, timedraw, time_font, GTextAlignmentCenter, FTextAnchorTop);
    fctx_end_fill(&fctx);

    fctx_deinit_context(&fctx);
}


static void layer_update_proc_zero(Layer * layer, GContext * ctx){
//draw battery on all layers
  GRect BatteryRect =
    (PBL_IF_ROUND_ELSE(
      GRect(0,144,180,2),
      GRect(0,0,144,3)));

  int s_battery_level = battery_state_service_peek().charge_percent;

    #ifdef PBL_ROUND
      int width_round = (s_battery_level * 180) / 100;
    #else
      int width_rect = (s_battery_level * 144) / 100;
    #endif

  GRect BatteryFillRect =
      (PBL_IF_ROUND_ELSE(
        GRect(18,144,width_round,2),
        GRect(0,0,width_rect,3)));

  char battperc[6];
  snprintf(battperc, sizeof(battperc), "%d", s_battery_level);
  strcat(battperc, "%");

  // Draw the battery bar & divider bar backgrounds
  graphics_context_set_fill_color(ctx, ColorSelect(settings.FrameColor2,settings.FrameColor2N));
  graphics_fill_rect(ctx, BatteryRect, 0, GCornerNone);

  // Draw the battery bar
  graphics_context_set_fill_color(ctx, ColorSelect(settings.Text6Color,settings.Text6ColorN));
  graphics_fill_rect(ctx,BatteryFillRect, 0, GCornerNone);

//show date

  GRect DayofWeekRect =
  (PBL_IF_ROUND_ELSE(
      GRect(0+2, 132, 98, 36),
      GRect(0+2, 130, 72*2, 36)));

      //Date
      // Local language
      const char * sys_locale = i18n_get_system_locale();
      char datedraw[10];
      fetchwday(s_weekday, sys_locale, datedraw);
      char datenow[10];
      snprintf(datenow, sizeof(datenow), "%s", datedraw);

      int daydraw;
      daydraw = s_day;
      char daynow[8];
      snprintf(daynow, sizeof(daynow), "%d", daydraw);

      char monthdraw[10];
      fetchmonth(s_month, sys_locale, monthdraw);
      char monthnow[10];
      snprintf(monthnow, sizeof(monthnow), "%s", monthdraw);

  if (!settings.HealthOff && step_data_is_available())
    {
      char daydate[18];
      snprintf(daydate, sizeof (daydate), "%s", s_current_steps_buffer);
      graphics_context_set_text_color(ctx, ColorSelect(settings.Text3Color,settings.Text3ColorN));
      graphics_draw_text(ctx, daydate, FontDayOfTheWeekShortName, DayofWeekRect, GTextOverflowModeWordWrap, PBL_IF_ROUND_ELSE(GTextAlignmentCenter,GTextAlignmentCenter), NULL);

    }
    else{
      char daydate[18];
      snprintf(daydate, sizeof (daydate), "%s %d %s", datedraw,daydraw, monthdraw);
      graphics_context_set_text_color(ctx, ColorSelect(settings.Text3Color,settings.Text3ColorN));
      graphics_draw_text(ctx, daydate, FontDayOfTheWeekShortName, DayofWeekRect, GTextOverflowModeWordWrap, PBL_IF_ROUND_ELSE(GTextAlignmentCenter,GTextAlignmentCenter), NULL);

    }
  }


static void layer_update_proc_bt(Layer * layer3, GContext * ctx3){

    GRect BTIconRect =
    (PBL_IF_ROUND_ELSE(
      GRect(90,4,24,20),
      GRect(144-47,168-20,47,20)));

 graphics_context_set_text_color(ctx3, ColorSelect(settings.Text5Color, settings.Text5ColorN));
 graphics_draw_text(ctx3, "z", FontBTQTIcons, BTIconRect, GTextOverflowModeFill,GTextAlignmentRight, NULL);

}

static void layer_update_proc_qt(Layer * layer4, GContext * ctx4){


  GRect QTIconRect =
    (PBL_IF_ROUND_ELSE(
      GRect(90-24,4,24,20),
      GRect(0,168-20,47,20)));

 quiet_time_icon();

 graphics_context_set_text_color(ctx4, ColorSelect(settings.Text5Color, settings.Text5ColorN));
 graphics_draw_text(ctx4, "\U0000E061", FontBTQTIcons, QTIconRect, GTextOverflowModeFill,GTextAlignmentLeft, NULL);

}

/////////////////////////////////////////
////Init: Handle Settings and Weather////
/////////////////////////////////////////
// Read settings from persistent storage
static void prv_load_settings(){
  // Load the default settings
  prv_default_settings();
  // Read settings from persistent storage, if they exist
  persist_read_data(SETTINGS_KEY, & settings, sizeof(settings));
}
// Save the settings to persistent storage
static void prv_save_settings(){
  persist_write_data(SETTINGS_KEY, & settings, sizeof(settings));

}
// Handle the response from AppMessage
static void prv_inbox_received_handler(DictionaryIterator * iter, void * context){
  //  Colours
  Tuple * sd1_color_t = dict_find(iter, MESSAGE_KEY_FrameColor2);
  if (sd1_color_t){
    settings.FrameColor2 = GColorFromHEX(sd1_color_t-> value -> int32);
  }
  Tuple * nsd1_color_t = dict_find(iter, MESSAGE_KEY_FrameColor2N);
  if (nsd1_color_t){
    settings.FrameColor2N = GColorFromHEX(nsd1_color_t-> value -> int32);
  }
   Tuple * sd2_color_t = dict_find(iter, MESSAGE_KEY_FrameColor);
  if (sd2_color_t){
    settings.FrameColor = GColorFromHEX(sd2_color_t-> value -> int32);
  }
  Tuple * nsd2_color_t = dict_find(iter, MESSAGE_KEY_FrameColorN);
  if (nsd2_color_t){
    settings.FrameColorN = GColorFromHEX(nsd2_color_t-> value -> int32);
  }
  ////////////
  Tuple * tx1_color_t = dict_find(iter, MESSAGE_KEY_Text1Color);
  if (tx1_color_t){
    settings.Text1Color = GColorFromHEX(tx1_color_t-> value -> int32);
  }
  Tuple * ntx1_color_t = dict_find(iter, MESSAGE_KEY_Text1ColorN);
  if (ntx1_color_t){
    settings.Text1ColorN = GColorFromHEX(ntx1_color_t-> value -> int32);
  }
  ///////////////////////////////
  Tuple * hr_color_t = dict_find(iter, MESSAGE_KEY_HourColor);
  if (hr_color_t){
    settings.HourColor = GColorFromHEX(hr_color_t-> value -> int32);
  }
  Tuple * nthr_color_t = dict_find(iter, MESSAGE_KEY_HourColorN);
  if (nthr_color_t){
    settings.HourColorN = GColorFromHEX(nthr_color_t-> value -> int32);
  }
  Tuple * min_color_t = dict_find(iter, MESSAGE_KEY_MinColor);
  if (min_color_t){
    settings.MinColor = GColorFromHEX(min_color_t-> value -> int32);
  }
  Tuple * ntmin_color_t = dict_find(iter, MESSAGE_KEY_MinColorN);
  if (ntmin_color_t){
    settings.MinColorN = GColorFromHEX(ntmin_color_t-> value -> int32);
  }
  Tuple * date_color_t = dict_find(iter, MESSAGE_KEY_DateColor);
  if (date_color_t){
    settings.DateColor = GColorFromHEX(date_color_t-> value -> int32);
  }
  Tuple * ntdate_color_t = dict_find(iter, MESSAGE_KEY_DateColorN);
  if (ntdate_color_t){
    settings.DateColorN = GColorFromHEX(ntdate_color_t-> value -> int32);
  }
  ///////////////////////////////
  Tuple * tx2_color_t = dict_find(iter, MESSAGE_KEY_Text2Color);
  if (tx2_color_t){
    settings.Text2Color = GColorFromHEX(tx2_color_t-> value -> int32);
  }
  Tuple * ntx2_color_t = dict_find(iter, MESSAGE_KEY_Text2ColorN);
  if (ntx2_color_t){
    settings.Text2ColorN = GColorFromHEX(ntx2_color_t-> value -> int32);
  }
   Tuple * tx3_color_t = dict_find(iter, MESSAGE_KEY_Text3Color);
  if (tx3_color_t){
    settings.Text3Color = GColorFromHEX(tx3_color_t-> value -> int32);
  }
  Tuple * ntx3_color_t = dict_find(iter, MESSAGE_KEY_Text3ColorN);
  if (ntx3_color_t){
    settings.Text3ColorN = GColorFromHEX(ntx3_color_t-> value -> int32);
  }
  Tuple * tx4_color_t = dict_find(iter,MESSAGE_KEY_Text4Color);
  if (tx4_color_t){
    settings.Text4Color = GColorFromHEX(tx4_color_t-> value -> int32);
    }
  Tuple * ntx4_color_t = dict_find(iter, MESSAGE_KEY_Text4ColorN);
  if(ntx4_color_t){
    settings.Text4ColorN = GColorFromHEX(ntx4_color_t-> value -> int32);
  }
  Tuple * tx5_color_t = dict_find(iter,MESSAGE_KEY_Text5Color);
  if (tx5_color_t){
    settings.Text5Color = GColorFromHEX(tx5_color_t-> value -> int32);
    }
  Tuple * ntx5_color_t = dict_find(iter, MESSAGE_KEY_Text5ColorN);
  if(ntx5_color_t){
    settings.Text5ColorN = GColorFromHEX(ntx5_color_t-> value -> int32);
  }
   Tuple * tx6_color_t = dict_find(iter,MESSAGE_KEY_Text6Color);
  if (tx6_color_t){
    settings.Text6Color = GColorFromHEX(tx6_color_t-> value -> int32);
    }
  Tuple * ntx6_color_t = dict_find(iter, MESSAGE_KEY_Text6ColorN);
  if(ntx6_color_t){
    settings.Text6ColorN = GColorFromHEX(ntx6_color_t-> value -> int32);
  }
  if (tx6_color_t){
    settings.Text6Color = GColorFromHEX(tx6_color_t-> value -> int32);
    }

  Tuple * tx7_color_t = dict_find(iter,MESSAGE_KEY_Text7Color);
 if (tx7_color_t){
   settings.Text7Color = GColorFromHEX(tx7_color_t-> value -> int32);
   }
 Tuple * ntx7_color_t = dict_find(iter, MESSAGE_KEY_Text7ColorN);
 if(ntx7_color_t){
   settings.Text7ColorN = GColorFromHEX(ntx7_color_t-> value -> int32);
 }
 if (tx7_color_t){
   settings.Text7Color = GColorFromHEX(tx7_color_t-> value -> int32);
   }

 Tuple * tx8_color_t = dict_find(iter,MESSAGE_KEY_Text8Color);
if (tx8_color_t){
  settings.Text8Color = GColorFromHEX(tx8_color_t-> value -> int32);
  }
Tuple * ntx8_color_t = dict_find(iter, MESSAGE_KEY_Text8ColorN);
if(ntx8_color_t){
  settings.Text8ColorN = GColorFromHEX(ntx8_color_t-> value -> int32);
}
if (tx8_color_t){
  settings.Text8Color = GColorFromHEX(tx8_color_t-> value -> int32);
  }

Tuple * tx9_color_t = dict_find(iter,MESSAGE_KEY_Text9Color);
if (tx9_color_t){
 settings.Text9Color = GColorFromHEX(tx9_color_t-> value -> int32);
 }
Tuple * ntx9_color_t = dict_find(iter, MESSAGE_KEY_Text9ColorN);
if(ntx9_color_t){
 settings.Text9ColorN = GColorFromHEX(ntx9_color_t-> value -> int32);
}
if (tx9_color_t){
 settings.Text9Color = GColorFromHEX(tx9_color_t-> value -> int32);
 }

Tuple * tx10_color_t = dict_find(iter,MESSAGE_KEY_Text10Color);
if (tx10_color_t){
settings.Text10Color = GColorFromHEX(tx10_color_t-> value -> int32);
}
Tuple * ntx10_color_t = dict_find(iter, MESSAGE_KEY_Text10ColorN);
if(ntx10_color_t){
settings.Text10ColorN = GColorFromHEX(ntx10_color_t-> value -> int32);
}
if (tx10_color_t){
settings.Text10Color = GColorFromHEX(tx10_color_t-> value -> int32);
}


  //config data

  Tuple * disntheme_t = dict_find(iter, MESSAGE_KEY_NightTheme);
  if (disntheme_t){
    if (disntheme_t -> value -> int32 == 0){
      settings.NightTheme = false;
      APP_LOG(APP_LOG_LEVEL_DEBUG, "NTHeme off");
    } else {
    settings.NightTheme = true;
    APP_LOG(APP_LOG_LEVEL_DEBUG, "NTHeme on");
      }
    }

  Tuple * health_t = dict_find(iter, MESSAGE_KEY_HealthOff);
  if (health_t){
    if (health_t -> value -> int32 == 0){
      settings.HealthOff = false;
      APP_LOG(APP_LOG_LEVEL_DEBUG, "Health on");
      get_step_count();
      display_step_count();
    } else {
      settings.HealthOff = true;
      APP_LOG(APP_LOG_LEVEL_DEBUG, "Health off");
      snprintf(s_current_steps_buffer, sizeof(s_current_steps_buffer),
       "%s", "");
    }
  }

  Tuple * vibe_t = dict_find(iter, MESSAGE_KEY_VibeOn);
  if (vibe_t){
    if (vibe_t -> value -> int32 == 0){
      settings.VibeOn = false;
      APP_LOG(APP_LOG_LEVEL_DEBUG, "Vibe off");
    } else {
      settings.VibeOn = true;
      APP_LOG(APP_LOG_LEVEL_DEBUG, "Vibe on");
    }
  }

  Tuple * addzero12_t = dict_find(iter, MESSAGE_KEY_AddZero12h);
  if (addzero12_t){
    if (addzero12_t -> value -> int32 == 0){
      settings.AddZero12h = false;
      APP_LOG(APP_LOG_LEVEL_DEBUG, "Add Zero 12h off");
    } else {
    settings.AddZero12h = true;
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Add Zero 12h on");
      }
    }

  Tuple * remzero24_t = dict_find(iter, MESSAGE_KEY_RemoveZero24h);
  if (remzero24_t){
    if (remzero24_t -> value -> int32 == 0){
      settings.RemoveZero24h = false;
      APP_LOG(APP_LOG_LEVEL_DEBUG, "Remove Zero 24h off");
    } else {
    settings.RemoveZero24h = true;
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Remove Zero 24h off");
      }
    }

  layer_mark_dirty(s_canvas_weather_section);
  layer_mark_dirty(time_area_layer);
  layer_mark_dirty(s_canvas_bt_icon);
  layer_mark_dirty(s_canvas_qt_icon);
  layer_mark_dirty(s_canvas_background);

  prv_save_settings();
}

//Load main layer
static void window_load(Window * window){
  Layer * window_layer = window_get_root_layer(window);
  GRect bounds4 = layer_get_bounds(window_layer);

  s_canvas_background = layer_create(bounds4);
    layer_set_update_proc(s_canvas_background, layer_update_proc_background);
    layer_add_child(window_layer, s_canvas_background);

  time_area_layer = layer_create(bounds4);
     layer_add_child(window_layer, time_area_layer);
     layer_set_update_proc(time_area_layer, update_time_area_layer);

  s_canvas_bt_icon = layer_create(bounds4);
    layer_set_update_proc (s_canvas_bt_icon, layer_update_proc_bt);
    layer_add_child(window_layer, s_canvas_bt_icon);

  s_canvas_qt_icon = layer_create(bounds4);
    layer_set_update_proc (s_canvas_qt_icon, layer_update_proc_qt);
    layer_add_child(window_layer, s_canvas_qt_icon);

  s_canvas_weather_section = layer_create(bounds4);
    layer_set_update_proc(s_canvas_weather_section, layer_update_proc_zero);
    layer_add_child(window_layer, s_canvas_weather_section);
  }


static void window_unload(Window * window){
  layer_destroy (s_canvas_background);
  layer_destroy(s_canvas_weather_section);
  layer_destroy(time_area_layer);
  layer_destroy(s_canvas_bt_icon);
  layer_destroy(s_canvas_qt_icon);
  window_destroy(s_window);

  ffont_destroy(time_font);
  fonts_unload_custom_font(FontDayOfTheWeekShortName);
  fonts_unload_custom_font(FontBTQTIcons);

}

void main_window_push(){
  s_window = window_create();
  window_set_window_handlers(s_window, (WindowHandlers){
    .load = window_load,
    .unload = window_unload,
  });
  window_stack_push(s_window, true);
}

void main_window_update(int hours, int minutes, int weekday, int day, int month){
  s_hours = hours;
  s_minutes = minutes;
  s_day = day;
  s_weekday = weekday;
  s_month = month;

  layer_mark_dirty(time_area_layer);
}

static void tick_handler(struct tm * time_now, TimeUnits changed){

  main_window_update(time_now -> tm_hour, time_now -> tm_min, time_now -> tm_wday, time_now -> tm_mday, time_now -> tm_mon);
  //update_time();
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Tick at %d", time_now -> tm_min);

  bluetooth_callback(connection_service_peek_pebble_app_connection());
}

static void init(){

  prv_load_settings();

  // Listen for AppMessages

    time_t now = time(NULL);
  struct tm *t = localtime(&now);
  s_hours=t->tm_hour;
  s_minutes=t->tm_min;
  s_day=t->tm_mday;
  s_weekday=t->tm_wday;
  s_month=t->tm_mon;
  //Register and open
  app_message_register_inbox_received(prv_inbox_received_handler);
  app_message_open(512,512);

// Load Fonts

  time_font =  ffont_create_from_resource(RESOURCE_ID_FFONT_ERETCOM);
  FontDayOfTheWeekShortName = fonts_load_custom_font(resource_get_handle(PBL_IF_ROUND_ELSE(RESOURCE_ID_ERET_36, RESOURCE_ID_ERET_36)));
  FontBTQTIcons = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_DRIPICONS_16));

  main_window_push();
  // Register with Event Services
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);

  if (!settings.HealthOff && step_data_is_available())  {
      health_service_events_subscribe(health_handler,NULL);
      APP_LOG(APP_LOG_LEVEL_DEBUG, "health is on, and steps data is subscribed");
    }

  connection_service_subscribe((ConnectionHandlers){
    .pebble_app_connection_handler = bluetooth_vibe_icon
  });
  bluetooth_vibe_icon(connection_service_peek_pebble_app_connection());

}
static void deinit(){
  tick_timer_service_unsubscribe();
  app_message_deregister_callbacks();
  battery_state_service_unsubscribe();
  connection_service_unsubscribe();
  health_service_events_unsubscribe();

}
int main(){
  init();
  app_event_loop();
  deinit();
}

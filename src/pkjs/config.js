// Clay Config: see https://github.com/pebble/clay
module.exports = [
  {
    "type": "heading",
    "defaultValue": "Settings"
  },
  {
    "type": "section",
    "items": [
      {
        "type": "toggle",
        "messageKey": "HealthOff",
        "label": "Switch off Health|Steps",
        "defaultValue": true,
        "capabilities":["HEALTH"]
      },
      {
        "type": "toggle",
        "messageKey": "VibeOn",
        "label": "Switch on vibrate on Bluetooth disconnect during Quiet Time",
        "defaultValue": false
      },
      {
        "type": "heading",
        "defaultValue": "Time Section",
        "size":4
      },
      {
        "type": "color",
        "messageKey": "FrameColor2",
        "defaultValue": "0x000000",
        "label": "Time Background Colour",
        "allowGray":true
      },
      {
        "type": "color",
        "messageKey": "FrameColor",
        "defaultValue": "0x000000",
        "label": "Complications Background Colour",
        "allowGray":true
      },
      {
        "type": "color",
        "messageKey": "HourColor",
        "defaultValue": "0xFFFFFF",
        "label": "Time Colour",
        "allowGray":true
      },
      {
        "type": "color",
        "messageKey": "Text3Color",
        "defaultValue": "0xFFFFFF",
        "label": "Date Colour",
        "allowGray":true
      },
      {
        "type": "color",
        "messageKey": "Text5Color",
        "defaultValue": "0xFFFFFF",
        "label": "Bluetooth Disconnect & Quiet Time Icons",
        "allowGray":true
      },
      {
        "type": "color",
        "messageKey": "Text6Color",
        "defaultValue": "0xFFFFFF",
        "label": "Battery Bar Colour",
        "allowGray":true
      },
      {
        "type": "toggle",
        "messageKey": "AddZero12h",
        "label": "Add leading zero to 12h time",
        "defaultValue": false
      },
      {
        "type": "toggle",
        "messageKey": "RemoveZero24h",
        "label": "Remove leading zero from 24h time",
        "defaultValue": false
      },
    ]
  },
          {
                "type": "submit",
                "defaultValue":"SAVE"
                },
                {
                "type": "heading",
                "defaultValue": "version v1.0",
                "size":6
                },
                {
                "type": "heading",
                "defaultValue": "Made in the UK",
                "size":6
                }
              ];

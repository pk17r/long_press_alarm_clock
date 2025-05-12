#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <string>

// SELECT MCU

#define MCU_IS_ESP32_S3
// #define MCU_IS_ESP32_S2_MINI
// #define MCU_IS_ESP32_WROOM_DA_MODULE


// SELECT DISPLAY

#define DISPLAY_IS_ST7789V
// #define DISPLAY_IS_ST7796
// #define DISPLAY_IS_ST7735
// #define DISPLAY_IS_ILI9341
// #define DISPLAY_IS_ILI9488


// SELECT IF WIFI IS USED

#define WIFI_IS_USED


// FIRMWARE VERSION   (update these when pushing new MCU specific binaries to github)

#define ESP32_S3_FIRMWARE_VERSION                 "v3.8"
#define ESP32_S2_MINI_FIRMWARE_VERSION            "v3.8"
#define ESP32_WROOM_DA_MODULE_FIRMWARE_VERSION    "2.4"
const std::string kFirmwareDate = "May 11, 2025";

const std::string kChangeLog = "- Adjust Min Screensaver\n  Brightness!\n- Software improvements.";


// #define MORE_LOGS


#endif  // CONFIGURATION_H
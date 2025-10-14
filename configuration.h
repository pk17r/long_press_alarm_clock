#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <string>

// #define MCU_IS_ESP32_S2
// #define MCU_IS_ESP32_S3
#define MCU_IS_ESP32_S2_MINI
// #define MCU_IS_ESP32_WROOM_DA_MODULE

#define WIFI_IS_USED

#define ESP32_S2_FIRMWARE_VERSION                 "v4.3.1"
#define ESP32_S3_FIRMWARE_VERSION                 "v4.3.1"
#define ESP32_S2_MINI_FIRMWARE_VERSION            "v4.3.1"
#define ESP32_WROOM_DA_MODULE_FIRMWARE_VERSION    "v4.2.1"
const std::string kFirmwareDate = "Oct 14, 2025";

const std::string kChangeLog = "- Busy Symbol, No-WiFi Symbol!\n- Weather & TimeZone Btn\n- UI & Software Enhancements";

// #define MORE_LOGS

#endif  // CONFIGURATION_H
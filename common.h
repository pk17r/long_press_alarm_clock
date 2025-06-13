#ifndef COMMON_H
#define COMMON_H

// common includes
#include <Arduino.h>
#include "pin_defs.h"
#include "general_constants.h"
#include <queue>          // std::queue
#include <vector>         // std::vector
#include "SPI.h"
#include <elapsedMillis.h>

#if defined(MCU_IS_ESP32_S3)
  const std::string kFirmwareVersion = ESP32_S3_FIRMWARE_VERSION;
  const std::string kFwSearchStr = "ESP32_S3_FIRMWARE_VERSION";
  #define CONFIG_FREERTOS_NUMBER_OF_CORES 2
#elif defined(MCU_IS_ESP32_S2_MINI)
  const std::string kFirmwareVersion = ESP32_S2_MINI_FIRMWARE_VERSION;
  const std::string kFwSearchStr = "ESP32_S2_MINI_FIRMWARE_VERSION";
  #define CONFIG_FREERTOS_NUMBER_OF_CORES 1       // watchdog timer usage https://iotassistant.io/esp32/fixing-error-hardware-wdt-arduino-esp32/
#elif defined(MCU_IS_ESP32_WROOM_DA_MODULE)
  const std::string kFirmwareVersion = ESP32_WROOM_DA_MODULE_FIRMWARE_VERSION;
  const std::string kFwSearchStr = "ESP32_WROOM_DA_MODULE_FIRMWARE_VERSION";
  #define CONFIG_FREERTOS_NUMBER_OF_CORES 2
#endif

// forward decleration of classes
class RTC;
class RGBDisplay;
class AlarmClock;
class WiFiStuff;
class NvsPreferences;
class PushButtonTaps;
class Touchscreen;

// spi
extern SPIClass* spi_obj;

// extern all global variables
extern RTC* rtc;
extern RGBDisplay* display;
extern AlarmClock* alarm_clock;
extern WiFiStuff* wifi_stuff;
extern NvsPreferences* nvs_preferences;
extern PushButtonTaps* push_button;
extern PushButtonTaps* inc_button;
extern PushButtonTaps* dec_button;
extern Touchscreen* ts;

// debug mode turned On by pulling debug pin Low
extern bool debug_mode;

extern bool use_photoresistor;

// CPU Speed for ESP32 CPU
extern uint8_t cpu_speed_mhz;

// firmware updated flag user information
extern bool firmware_updated_flag_user_information;

// counter to note user inactivity seconds
extern elapsedMillis inactivity_millis;

extern void TurnOnRgbStrip();
extern void TurnOffRgbStrip();
extern bool rgb_led_strip_on;
enum RgbLedMode {
  RGB_LED_MANUAL_OFF,
  RGB_LED_MANUAL_ON,
  RGB_LED_AUTO_EVENING,
  RGB_LED_AUTO_SUN_DOWN
};
extern RgbLedMode autorun_rgb_led_strip_mode;
extern int current_rgb_led_strip_index;
extern uint16_t night_time_minutes;

// flag for display pages
enum ScreenPage {
  kMainPage = 0,
  kScreensaverPage,
  kScreensaverSettingsPage,
  kRgbLedSettingsPage,
  kAlarmSetPage,
  kAlarmTriggeredPage,
  kTimeSetPage,
  kSettingsPage,
  kWiFiSettingsPage,
  kWiFiScanNetworksPage,
  kSoftApInputsPage,
  kClockSettingsPage,
  kDisplaySettingsPage,
  kWeatherSettingsPage,
  kLocationInputsLocalServerPage,
  kFirmwareUpdatePage,
  kNoPageSelected
  };

// current page on display
extern ScreenPage current_page;

// flag for cursor highlight location
enum Cursor {
  kCursorNoSelection = 0,
  kMainPageSettingsWheel,
  kMainPageSetAlarm,
  kAlarmSetPageHour,
  kAlarmSetPageMinute,
  kAlarmSetPageAmPm,
  kAlarmSetPageOn,
  kAlarmSetPageOff,
  kAlarmSetPageSet,
  kAlarmSetPageCancel,
  kSettingsPageWiFi,
  kSettingsPageClock,
  kSettingsPageScreensaver,
  kSettingsPageRgbLed,
  kSettingsPageWeather,
  kWiFiSettingsPageShowSsidRow,
  kWiFiSettingsPageScanNetworks,
  kWiFiSettingsPageChangePasswd,
  kWiFiSettingsPageClearSsidAndPasswd,
  kWiFiSettingsPageConnect,
  kWiFiSettingsPageDisconnect,
  kWiFiScanNetworksPageList,
  kWiFiScanNetworksPageRescan,
  kWiFiScanNetworksPageNext,
  kClockSettingsPageOwnerName,
  kClockSettingsPageSetLocation,
  kClockSettingsPageUpdateTime,
  kClockSettingsPageAlarmLongPressTime,
  kClockSettingsPageDisplaySettings,
  kClockSettingsPageUpdateFirmware,
  kDisplaySettingsPageRotateScreen,
  kDisplaySettingsPageTestTouchscreenCalibration,
  kDisplaySettingsPageCalibrateTouchscreen,
  kWeatherSettingsPageSetUnits,
  kWeatherSettingsPageFetch,
  kScreensaverSettingsPageMotion,
  kScreensaverSettingsPageSpeed,
  kScreensaverSettingsPageRun,
  kScreensaverSettingsPageNightTimeColorChange,
  kScreensaverSettingsPageMinBrightness,
  kRgbLedSettingsPageNightTmDimHr,
  kRgbLedSettingsPageRgbLedStripMode,
  kRgbLedSettingsPageRgbLedBrightness,
  kPageSaveButton,
  kPageBackButton,
  kCursorMaxValue,    // inc/dec button scroll won't go above this level
  };

// current cursor highlight location on page
extern Cursor current_cursor;

// postfix form ++ -- operator overloads for Cursor enum variables
inline Cursor operator++ (Cursor& highlight_location, int) {
  if(static_cast<int>(highlight_location) < static_cast<int>(kCursorMaxValue) - 1)
    highlight_location = static_cast<Cursor>(static_cast<int>(highlight_location) + 1);
  else
    highlight_location = kCursorNoSelection;
  return highlight_location;
}
inline Cursor operator-- (Cursor& highlight_location, int) {
  if(static_cast<int>(highlight_location) > static_cast<int>(kCursorNoSelection))
    highlight_location = static_cast<Cursor>(static_cast<int>(highlight_location) - 1);
  else
    highlight_location = static_cast<Cursor>(static_cast<int>(kCursorMaxValue) - 1);
  return highlight_location;
}


// flag for second core task
enum SecondCoreTask {
  kStartSetWiFiSoftAP = 0,
  kStopSetWiFiSoftAP,
  kScanNetworks,
  kStartLocationInputsLocalServer,
  kStopLocationInputsLocalServer,
  kGetWeatherInfo,
  kUpdateTimeFromNtpServer,
  kConnectWiFi,
  kDisconnectWiFi,
  kFirmwareVersionCheck,
  kNoTask    // needs to be last entry ibn the enum -> used to create second_core_task_added_flag_array
};

// second core current task
// extern volatile SecondCoreTask second_core_task;
extern std::queue<SecondCoreTask> second_core_tasks_queue;
extern bool second_core_task_added_flag_array[];


// Display Items

enum ButtonType {
  kClickButtonWithIcon,
  kClickButtonWithLabel,
  kLabelOnlyNoClickButton,    // kLabelOnlyNoClickButton will not be highlighted on display
};

// struct declerations
struct DisplayButton {
  const Cursor btn_cursor_id;
  const ButtonType btn_type;
  const std::string row_label;
  const bool fixed_location;
  int16_t btn_x;
  int16_t btn_y;
  uint16_t btn_w;
  uint16_t btn_h;
  std::string btn_value;
};

extern std::vector<std::vector<DisplayButton*>> display_pages_vec;

// display time data in char arrays
struct DisplayData {
  char time_HHMM[kHHMM_ArraySize];
  char time_SS[kSS_ArraySize];
  char date_str[kDateArraySize];
  char alarm_str[kAlarmArraySize];
  bool _12_hour_mode;
  bool pm_not_am;
  bool alarm_ON;
};

// Display Visible Data Struct
extern DisplayData new_display_data_, displayed_data_;

// extern all global functions
extern void AddSecondCoreTaskIfNotThere(SecondCoreTask task);
extern void WaitForExecutionOfSecondCoreTask();
extern int AvailableRam();
extern int MinFreeRam();
extern void SerialInputWait();
extern void SerialInputFlush();
extern void SerialTimeStampPrefix();
extern void PrepareTimeDayDateArrays();
extern void SerialPrintRtcDateTime();
extern void SerialUserInput();
extern void CycleCpuFrequency();
extern void SetPage(ScreenPage set_this_page, bool move_cursor_to_first_button, bool increment_page);
extern void SetPage(ScreenPage set_this_page, bool move_cursor_to_first_button);
extern void SetPage(ScreenPage set_this_page);
extern void SetWatchdogTime(unsigned long ms);
extern void ResetWatchdog();
extern void PrintLn(const char* someText1, const char* someText2);
extern void PrintLn(const char* someText1);
extern void PrintLn(const char* someText1, int someInt);
extern void PrintLn(std::string someTextStr1, std::string someTextStr2);
extern void PrintLn(std::string &someTextStr1, std::string &someTextStr2);
extern void PrintLn(std::string &someTextStr1, int someInt);
extern void PrintLn(std::string &someTextStr1);
extern void PrintLn(int someInt);
extern void PrintLn();
extern bool AnyButtonPressed();
extern void LedFeedback(bool value);
extern void LedFeedbackOnOff();

#endif // COMMON_H
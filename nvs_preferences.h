#ifndef NVS_PREFERENCES_H
#define NVS_PREFERENCES_H
/*
  Preferences is Arduino EEPROM replacement library using ESP32's On-Board Non-Volatile Memory
*/

#include <Preferences.h> //https://github.com/espressif/arduino-esp32/tree/master/libraries/Preferences
#include "common.h"
#include "secrets.h"

class NvsPreferences {

public:

  NvsPreferences();

  void RetrieveOwnerName(std::string &owner_name);
  void SaveOwnerName(std::string owner_name);
  void RetrieveLongPressSeconds(uint8_t &long_press_seconds);
  void SaveLongPressSeconds(uint8_t long_press_seconds);
  void RetrieveBuzzerFrequency(uint16_t &buzzer_freq);
  void SaveBuzzerFrequency(uint16_t buzzer_freq);
  void RetrieveAlarmSettings(uint8_t &alarmHr, uint8_t &alarmMin, bool &alarmIsAm, bool &alarmOn);
  void SaveAlarm(uint8_t alarmHr, uint8_t alarmMin, bool alarmIsAm, bool alarmOn);
  void RetrieveWiFiDetails(std::string &wifi_ssid, std::string &wifi_password);
  void SaveWiFiDetails(std::string wifi_ssid, std::string wifi_password);
  void RetrieveLocationDetails(std::string &location_zip_code, std::string &location_country_code, std::string &city_name);
  void SaveLocationDetails(std::string location_zip_code, std::string location_country_code, std::string city_name);
  bool RetrieveWeatherUnits();
  void SaveWeatherUnits(bool weather_units_metric_not_imperial);
  std::string RetrieveSavedFirmwareVersion();
  void SaveCurrentFirmwareVersion();
  uint8_t RetrieveSavedCpuSpeed();
  void SaveCpuSpeed();
  bool RetrieveScreensaverBounceNotFlyHorizontally();
  void SaveScreensaverBounceNotFlyHorizontally(bool screensaverBounceNotFlyHorizontally);
  void SaveScreensaverSleepFriendNightColor(bool sleep_friendly_color_at_night);
  bool RetrieveScreensaverSleepFriendNightColor();
  uint8_t RetrieveNightTimeDimHour();
  void SaveNightTimeDimHour(uint8_t night_time_dim_hour);
  uint8_t RetrieveScreenOrientation();
  void SaveScreenOrientation(uint8_t screen_orientation);
  uint8_t RetrieveAutorunRgbLedStripMode();
  void SaveAutorunRgbLedStripMode(uint8_t autorun_rgb_led_strip_mode_to_save);
  bool RetrieveUseLdr();
  void SaveUseLdr(bool use_ldr);
  void RetrieveTestVal(uint8_t &test_val);
  uint8_t RetrieveTestValOrSaveDefault();
  void SaveTestVal(uint8_t test_val);
  uint8_t RetrieveTouchscreenType();
  void SaveTouchscreenType(uint8_t touchscreen_type);
  bool RetrieveTouchscreenFlip();
  void SaveTouchscreenFlip(bool touchscreen_flip);
  uint8_t RetrieveRgbStripLedCount();
  void SaveRgbStripLedCount(uint8_t rgb_strip_led_count);
  uint8_t RetrieveRgbStripLedBrightness();
  void SaveRgbStripLedBrightness(uint8_t rgb_strip_led_brightness);
  uint8_t RetrieveRtcType();
  void SaveRtcType(uint8_t rtc_type);
  void RemoveKey(std::string remove_key);
  void RetrieveTouchScreenCalibration(int16_t &xMin, int16_t &xMax, int16_t &yMin, int16_t &yMax);
  void SaveTouchScreenCalibration(int16_t xMin, int16_t xMax, int16_t yMin, int16_t yMax);
  uint8_t RetrieveHwVersion();
  void RetrieveCityName(std::string &city_name);
  void SaveCityName(std::string city_name);
  uint8_t RetrieveDisplayMinBrightnessAdder();
  void SaveDisplayMinBrightnessAdder(uint8_t display_min_brightness_adder);

private:

  // ESP32 NVS Memory Data Access     ***** MAX KEY LENGTH 15 CHARACTERS *****

  Preferences preferences;

  const char* kNvsDataKey = "longPressData";

  /*      HARDWARE VERSION
   *  Sets My_Hw_Version and controls Pin Selection in pin_defs.h for following hardwares:
   *  ESP32 S3 HW Versions: 1
   *  ESP32 S2 HW Versions: 1
   *  ESP32 DevKit HW Versions: 1
   */
  const char* kHwVersionKey = "HwVersion";
  #if defined(MCU_IS_ESP32_S3)
    const uint8_t kHwVersion = 0x01;
  #elif defined(MCU_IS_ESP32_S2_MINI)
    const uint8_t kHwVersion = 0x01;
  #elif defined(MCU_IS_ESP32_WROOM_DA_MODULE)
    const uint8_t kHwVersion = 0x01;
  #endif

  const char* kOwnerNameKey = "OwnerName";
  const std::string kOwnerName = "<name>";

  const char* kCityNameKey = "City";
  const std::string kCityName = "";

  const char* kAlarmHrKey = "AlarmHr";
  const uint8_t kAlarmHr = 7;

  const char* kAlarmMinKey = "AlarmMin";
  const uint8_t kAlarmMin = 30;

  const char* kAlarmIsAmKey = "AlarmIsAm";
  const bool kAlarmIsAm = true;

  const char* kAlarmOnKey = "AlarmOn";
  const bool kAlarmOn = false;

  const char* kWiFiSsidKey = "WiFiSsid";  // kWifiSsidPasswordLengthMax bytes
  #if defined(MY_WIFI_SSID)   // create a secrets.h file with #define for MY_WIFI_SSID and uncomment the include statement at top of this file
    std::string kWiFiSsid = MY_WIFI_SSID;
  #else
    std::string kWiFiSsid = "Enter SSID";
  #endif

  const char* kWiFiPasswdKey = "WiFiPasswd"; // kWifiSsidPasswordLengthMax bytes
  #if defined(MY_WIFI_PASSWD)   // create a secrets.h file with #define for MY_WIFI_PASSWD and uncomment the include statement at top of this file
    std::string kWiFiPasswd = MY_WIFI_PASSWD;
  #else
    std::string kWiFiPasswd = "Enter Passwd";
  #endif

  const char* kWeatherZipCodeNewKey = "LocationZip";
  const char* kWeatherZipCodeOldKey = "WeatherZip";
  const std::string kWeatherZipCode = "92104";

  const char* kWeatherCountryCodeKey = "WeatherCC";
  const std::string kWeatherCountryCode = "US";

  const char* kWeatherUnitsMetricNotImperialKey = "WeatherUnits";
  const bool kWeatherUnitsMetricNotImperial = false;

  const char* kAlarmLongPressSecondsKey = "AlarmLongPrsSec";
  const uint8_t kAlarmLongPressSeconds = 15;

  const char* kBuzzerFrequencyKey = "BuzzerFreq";
  const uint16_t kBuzzerFrequency = 2731;       // older selection of 12085 through hole buzzer had 2048Hz rated frequency. New selection of KLJ-7525-5027 SMD buzzer has 2731Hz as rated frequency.

  const char* kFirmwareVersionKey = "FwVersion";  // 6 bytes

  const char* kCpuSpeedMhzKey = "CpuSpeedMhz";  // 1 byte

  const char* kScreensaverMotionTypeKey = "ScSvrMotionTy";
  const bool kScreensaverMotionType = true;

  const char* kScreensaverSleepFriendlyColorAtNightKey = "ScSvrSlpColor";
  const bool kScreensaverSleepFriendlyColorAtNight = true;

  const char* kUseLDRKey = "UseLDR";
  const bool kUseLDR = true;

  const char* kDisplayMinBrightnessAdderKey = "DispMinBright";
  const uint8_t kDisplayMinBrightnessAdder = 2;

  const char* kNightTimeDimHourKey = "NightTmDimHour";
  const uint8_t kNightTimeDimHour = 10;

  const char* kAutorunRgbLedStripModeKey = "RgbLedStripMode";
  const uint8_t kAutorunRgbLedStripMode = 2;

  const char* kRgbStripLedCountKey = "RgbLedCount";
  const uint8_t kRgbStripLedCount = 4;

  const char* kRgbStripLedBrightnessKey = "RgbLedBright";
  const uint8_t kRgbStripLedBrightness = 255;

  const char* kScreenOrientationKey = "ScreenOrient";
  const uint8_t kScreenOrientation = 3;

  const char* kTouchscreenTypeKey = "Touchscreen";
  const uint8_t kTouchscreenType = 1;       // 0 = no touch, 1 = Resistive read using XPT2046, 2 = Resistive read using MCU ADC

  const char* kTouchscreenFlipKey = "TouchFlip";
  const bool kTouchscreenFlip = true;

  // touchscreen calibration
  // if(touchscreen_type == 1) // XPT2046     232, 3686, 221, 3767
  // if(touchscreen_type == 2) // MCU ADC     41, 881, 113, 914
  const char* kTouchCalibXMinKey = "TouchCalibXMin";
  const int16_t kTouchCalibXMin = (kTouchscreenType == 1 ? 232 : 41);
  const char* kTouchCalibXMaxKey = "TouchCalibXMax";
  const int16_t kTouchCalibXMax = (kTouchscreenType == 1 ? 3686 : 881);
  const char* kTouchCalibYMinKey = "TouchCalibYMin";
  const int16_t kTouchCalibYMin = (kTouchscreenType == 1 ? 221 : 113);
  const char* kTouchCalibYMaxKey = "TouchCalibYMax";
  const int16_t kTouchCalibYMax = (kTouchscreenType == 1 ? 3767 : 914);

  const char* kRtcTypeKey = "RtcType";
  const uint8_t kRtcType = 2;     // 1 = URTCLIB_MODEL_DS1307, 2 = URTCLIB_MODEL_DS3231

};

#endif  // NVS_PREFERENCES_H
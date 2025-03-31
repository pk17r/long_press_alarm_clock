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
  void RetrieveWeatherLocationForWiFiStuff(std::string &location_zip_code, std::string &location_country_code, bool &weather_units_metric_not_imperial);
  void SaveWeatherLocationFromWiFiStuff(std::string location_zip_code, std::string location_country_code, bool weather_units_metric_not_imperial);
  void SaveWeatherUnits(bool weather_units_metric_not_imperial);
  std::string RetrieveSavedFirmwareVersion();
  void SaveCurrentFirmwareVersion();
  uint8_t RetrieveSavedCpuSpeed();
  void SaveCpuSpeed();
  bool RetrieveScreensaverBounceNotFlyHorizontally();
  void SaveScreensaverBounceNotFlyHorizontally(bool screensaverBounceNotFlyHorizontally);
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

private:

  // ESP32 NVS Memory Data Access     ***** MAX KEY LENGTH 15 CHARACTERS *****

  Preferences preferences;

  const char* kNvsDataKey = "longPressData";

  const char* kOwnerNameKey = "OwnerName";
  const std::string kOwnerName = "?";

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

  const char* kNightTimeDimHourKey = "NightTmDimHour";
  const uint8_t kNightTimeDimHour = 10;

  const char* kAutorunRgbLedStripModeKey = "RgbLedStripMode";
  const uint8_t kAutorunRgbLedStripMode = 2;

  const char* kScreenOrientationKey = "ScreenOrient";
  const uint8_t kScreenOrientation = 3;

  const char* kUseLDRKey = "UseLDR";
  const bool kUseLDR = true;

  const char* kTouchscreenTypeKey = "Touchscreen";
  const uint8_t kTouchscreenType = 1;       // 0 = no touch, 1 = Resistive read using XPT2046, 2 = Resistive read using MCU ADC

  const char* kTouchscreenFlipKey = "TouchFlip";
  const bool kTouchscreenFlip = true;

  const char* kRgbStripLedCountKey = "RgbLedCount";
  const uint8_t kRgbStripLedCount = 4;

  const char* kRgbStripLedBrightnessKey = "RgbLedBright";
  const uint8_t kRgbStripLedBrightness = 255;

  const char* kRtcTypeKey = "RtcType";
  const uint8_t kRtcType = 2;     // 1 = URTCLIB_MODEL_DS1307, 2 = URTCLIB_MODEL_DS3231

};

#endif  // NVS_PREFERENCES_H
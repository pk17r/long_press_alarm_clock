#include "nvs_preferences.h"

NvsPreferences::NvsPreferences() {

  preferences.begin(kNvsDataKey, /*readOnly = */ false);

  // save key values
  // ADD NEW KEYS HERE
  if(!preferences.isKey(kAlarmHrKey))
    preferences.putUChar(kAlarmHrKey, kAlarmHr);
  if(!preferences.isKey(kAlarmMinKey))
    preferences.putUChar(kAlarmMinKey, kAlarmMin);
  if(!preferences.isKey(kAlarmIsAmKey))
    preferences.putBool(kAlarmIsAmKey, kAlarmIsAm);
  if(!preferences.isKey(kAlarmOnKey))
    preferences.putBool(kAlarmOnKey, kAlarmOn);
  if(!preferences.isKey(kWiFiSsidKey)) {
    String kWiFiSsidString = kWiFiSsid.c_str();
    preferences.putString(kWiFiSsidKey, kWiFiSsidString);
  }
  if(!preferences.isKey(kWiFiPasswdKey)) {
    String kWiFiPasswdString = kWiFiPasswd.c_str();
    preferences.putString(kWiFiPasswdKey, kWiFiPasswdString);
  }
  // switch between old and new Zip Code format
  if(preferences.isKey(kWeatherZipCodeOldKey)) {
    uint32_t location_zip_code = preferences.getUInt(kWeatherZipCodeOldKey);
    std::string location_zip_code_str = std::to_string(location_zip_code);
    while(location_zip_code_str.size() < 5) {
      location_zip_code_str = '0' + location_zip_code_str;
    }
    PrintLn("Switch Old Zip to New =", location_zip_code_str);
    String kWeatherZipCodeString = location_zip_code_str.c_str();
    preferences.putString(kWeatherZipCodeNewKey, kWeatherZipCodeString);
    preferences.remove(kWeatherZipCodeOldKey);
  }
  if(!preferences.isKey(kWeatherZipCodeNewKey)) {
    String kWeatherZipCodeString = kWeatherZipCode.c_str();
    preferences.putString(kWeatherZipCodeNewKey, kWeatherZipCodeString);
  }
  if(!preferences.isKey(kWeatherCountryCodeKey)) {
    String kWeatherCountryCodeString = kWeatherCountryCode.c_str();
    preferences.putString(kWeatherCountryCodeKey, kWeatherCountryCodeString);
  }
  if(!preferences.isKey(kWeatherUnitsMetricNotImperialKey))
    preferences.putBool(kWeatherUnitsMetricNotImperialKey, kWeatherUnitsMetricNotImperial);
  if(!preferences.isKey(kAlarmLongPressSecondsKey))
    preferences.putUChar(kAlarmLongPressSecondsKey, kAlarmLongPressSeconds);
  if(!preferences.isKey(kFirmwareVersionKey)) {
    String kFirmwareVersionString = kFirmwareVersion.c_str();
    preferences.putString(kFirmwareVersionKey, kFirmwareVersionString);
  }
  else {
    // a firmware is already present on device.
    // means device can have older 12085 buzzer with rated frequency of 2048 Hz
    // if already a firmware is present on device, then it can have either the older 12085 buzzer or newer SMD buzzer with a different frequency
    // firmware with 12085 buzzer did not have a buzzer frequency key. kBuzzerFrequencyKey was added when newer SMD buzzer was introduced.
    // devices with newer SMD buzzer, already have kBuzzerFrequencyKey defined.
    if(!preferences.isKey(kBuzzerFrequencyKey))   // if kBuzzerFrequencyKey is not present then this HW has older buzzer
      preferences.putUShort(kBuzzerFrequencyKey, 2048);   // dont change 2048 magic number here
    // if device already has a firmware, then it is HW1 device
    if(!preferences.isKey(kHwVersionKey))
      preferences.putUChar(kHwVersionKey, 0x01);
  }
  if(!preferences.isKey(kBuzzerFrequencyKey))
    preferences.putUShort(kBuzzerFrequencyKey, (kHwVersion == 0x01 ? kBuzzerFrequency_HW1 : kBuzzerFrequency_HW2));
  if(!preferences.isKey(kHwVersionKey))
    preferences.putUChar(kHwVersionKey, kHwVersion);
  if(!preferences.isKey(kCpuSpeedMhzKey))
    preferences.putUChar(kCpuSpeedMhzKey, cpu_speed_mhz);
  if(!preferences.isKey(kScreensaverMotionTypeKey))
    preferences.putBool(kScreensaverMotionTypeKey, kScreensaverMotionType);
  if(!preferences.isKey(kNightTimeDimHourKey))
    preferences.putUChar(kNightTimeDimHourKey, kNightTimeDimHour);
  if(!preferences.isKey(kScreenOrientationKey))
    preferences.putUChar(kScreenOrientationKey, kScreenOrientation);
  if(!preferences.isKey(kAutorunRgbLedStripModeKey))
    preferences.putUChar(kAutorunRgbLedStripModeKey, kAutorunRgbLedStripMode);
  if(!preferences.isKey(kUseLDRKey))
    preferences.putBool(kUseLDRKey, kUseLDR);
  if(!preferences.isKey(kTouchscreenTypeKey))
    preferences.putUChar(kTouchscreenTypeKey, kTouchscreenType);
  if(!preferences.isKey(kTouchscreenFlipKey))
    preferences.putBool(kTouchscreenFlipKey, kTouchscreenFlip);
  if(!preferences.isKey(kRgbStripLedCountKey))
    preferences.putUChar(kRgbStripLedCountKey, kRgbStripLedCount);
  if(!preferences.isKey(kRgbStripLedBrightnessKey))
    preferences.putUChar(kRgbStripLedBrightnessKey, kRgbStripLedBrightness);
  if(!preferences.isKey(kRtcTypeKey))
    preferences.putUChar(kRtcTypeKey, kRtcType);
  if(!preferences.isKey(kOwnerNameKey)) {
    String kOwnerNameString = kOwnerName.c_str();
    preferences.putString(kOwnerNameKey, kOwnerNameString);
  }
  if(!preferences.isKey(kTouchCalibXMinKey))
    preferences.putShort(kTouchCalibXMinKey, kTouchCalibXMin);
  if(!preferences.isKey(kTouchCalibXMaxKey))
    preferences.putShort(kTouchCalibXMaxKey, kTouchCalibXMax);
  if(!preferences.isKey(kTouchCalibYMinKey))
    preferences.putShort(kTouchCalibYMinKey, kTouchCalibYMin);
  if(!preferences.isKey(kTouchCalibYMaxKey))
    preferences.putShort(kTouchCalibYMaxKey, kTouchCalibYMax);
  if(!preferences.isKey(kScreensaverSleepFriendlyColorAtNightKey))
    preferences.putBool(kScreensaverSleepFriendlyColorAtNightKey, kScreensaverSleepFriendlyColorAtNight);
  if(!preferences.isKey(kCityNameKey)) {
    String kCityNameString = kCityName.c_str();
    preferences.putString(kCityNameKey, kCityNameString);
  }
  if(!preferences.isKey(kDisplayMinBrightnessAdderKey))
    preferences.putUChar(kDisplayMinBrightnessAdderKey, kDisplayMinBrightnessAdder);


  // save new key values
  // ADD NEW KEYS ABOVE
  preferences.end();

  // retrieve HW Version for pin definitions based on HW version
  My_Hw_Version = RetrieveHwVersion();

  PrintLn(__func__, kInitializedStr);
}

void NvsPreferences::RetrieveLongPressSeconds(uint8_t &long_press_seconds) {
  preferences.begin(kNvsDataKey, /*readOnly = */ true);
  long_press_seconds = preferences.getUChar(kAlarmLongPressSecondsKey, kAlarmLongPressSeconds);
  preferences.end();
}

void NvsPreferences::SaveLongPressSeconds(uint8_t long_press_seconds) {
  preferences.begin(kNvsDataKey, /*readOnly = */ false);
  preferences.putUChar(kAlarmLongPressSecondsKey, long_press_seconds);
  preferences.end();
  PrintLn(__func__, long_press_seconds);
}

void NvsPreferences::RetrieveBuzzerFrequency(uint16_t &buzzer_freq) {
  preferences.begin(kNvsDataKey, /*readOnly = */ true);
  buzzer_freq = preferences.getUShort(kBuzzerFrequencyKey);
  preferences.end();
  PrintLn(__func__, buzzer_freq);
}

void NvsPreferences::SaveBuzzerFrequency(uint16_t buzzer_freq) {
  preferences.begin(kNvsDataKey, /*readOnly = */ false);
  preferences.putUShort(kBuzzerFrequencyKey, buzzer_freq);
  preferences.end();
  PrintLn(__func__, buzzer_freq);
}

void NvsPreferences::RetrieveAlarmSettings(uint8_t &alarmHr, uint8_t &alarmMin, bool &alarmIsAm, bool &alarmOn) {
  preferences.begin(kNvsDataKey, /*readOnly = */ true);
  alarmHr = preferences.getUChar(kAlarmHrKey);
  alarmMin = preferences.getUChar(kAlarmMinKey);
  alarmIsAm = preferences.getBool(kAlarmIsAmKey);
  alarmOn = preferences.getBool(kAlarmOnKey);
  preferences.end();
  PrintLn(__func__, (std::to_string(alarmHr) + ":" + std::to_string(alarmMin) + " " + (alarmIsAm ? kAmLabel : kPmLabel) + " " + (alarmOn ? "ON" : "OFF")));
}

void NvsPreferences::SaveAlarm(uint8_t alarmHr, uint8_t alarmMin, bool alarmIsAm, bool alarmOn) {
  preferences.begin(kNvsDataKey, /*readOnly = */ false);
  preferences.putUChar(kAlarmHrKey, alarmHr);
  preferences.putUChar(kAlarmMinKey, alarmMin);
  preferences.putBool(kAlarmIsAmKey, alarmIsAm);
  preferences.putBool(kAlarmOnKey, alarmOn);
  preferences.end();
  // Serial.printf("NVS Memory SaveAlarm %2d:%02d alarmIsAm=%d alarmOn=%d\n", alarmHr, alarmMin, alarmIsAm, alarmOn);
  PrintLn(__func__, (std::to_string(alarmHr) + ":" + std::to_string(alarmMin) + " " + (alarmIsAm ? kAmLabel : kPmLabel) + " " + (alarmOn ? "ON" : "OFF")));
}

void NvsPreferences::RetrieveWiFiDetails(std::string &wifi_ssid, std::string &wifi_password) {
  preferences.begin(kNvsDataKey, /*readOnly = */ true);
  String kWiFiSsidString = preferences.getString(kWiFiSsidKey);
  String kWiFiPasswdString = preferences.getString(kWiFiPasswdKey);
  preferences.end();
  wifi_ssid = kWiFiSsidString.c_str();
  wifi_password = kWiFiPasswdString.c_str();
  // PrintLn("NVS Memory wifi_password: ", wifi_password.c_str());
  // PrintLn("WiFi details retrieved from NVS Memory.");
  PrintLn(__func__, wifi_ssid);
}

void NvsPreferences::SaveWiFiDetails(std::string wifi_ssid, std::string wifi_password) {
  preferences.begin(kNvsDataKey, /*readOnly = */ false);
  String kWiFiSsidString = wifi_ssid.c_str();
  preferences.putString(kWiFiSsidKey, kWiFiSsidString);
  String kWiFiPasswdString = wifi_password.c_str();
  preferences.putString(kWiFiPasswdKey, kWiFiPasswdString);
  preferences.end();
  // PrintLn("NVS Memory wifi_ssid: ", wifi_ssid.c_str());
  // PrintLn("NVS Memory wifi_password: ", wifi_password.c_str());
  PrintLn(__func__, wifi_ssid);
}

std::string NvsPreferences::RetrieveSavedFirmwareVersion() {
  preferences.begin(kNvsDataKey, /*readOnly = */ true);
  String savedFirmwareVersionString = preferences.getString(kFirmwareVersionKey);
  preferences.end();
  std::string savedFirmwareVersion = savedFirmwareVersionString.c_str();
  PrintLn(__func__, savedFirmwareVersion);
  return savedFirmwareVersion;
}

void NvsPreferences::SaveCurrentFirmwareVersion() {
  preferences.begin(kNvsDataKey, /*readOnly = */ false);
  String kFirmwareVersionString = kFirmwareVersion.c_str();
  preferences.putString(kFirmwareVersionKey, kFirmwareVersionString);
  preferences.end();
  // PrintLn("Current Firmware Version written to NVS Memory");
  PrintLn(__func__, kFirmwareVersion);
}

void NvsPreferences::RetrieveLocationDetails(std::string &location_zip_code, std::string &location_country_code, std::string &city_name) {
  preferences.begin(kNvsDataKey, /*readOnly = */ true);
  String location_zip_code_String = preferences.getString(kWeatherZipCodeNewKey);
  location_zip_code = location_zip_code_String.c_str();
  String location_country_code_String = preferences.getString(kWeatherCountryCodeKey);
  location_country_code = location_country_code_String.c_str();
  String city_name_string = preferences.getString(kCityNameKey);
  city_name = city_name_string.c_str();
  preferences.end();
  PrintLn(__func__, (location_zip_code + " " + location_country_code + " " + city_name));
}

void NvsPreferences::SaveLocationDetails(std::string location_zip_code, std::string location_country_code, std::string city_name) {
  preferences.begin(kNvsDataKey, /*readOnly = */ false);
  String kWeatherZipCodeString = location_zip_code.c_str();
  preferences.putString(kWeatherZipCodeNewKey, kWeatherZipCodeString);
  String kWeatherCountryCodeString = location_country_code.c_str();
  preferences.putString(kWeatherCountryCodeKey, kWeatherCountryCodeString);
  String city_name_string = city_name.c_str();
  preferences.putString(kCityNameKey, city_name_string);
  preferences.end();
  PrintLn(__func__, (location_zip_code + " " + location_country_code + " " + city_name));
}

bool NvsPreferences::RetrieveWeatherUnits() {
  preferences.begin(kNvsDataKey, /*readOnly = */ true);
  bool weather_units_metric_not_imperial = preferences.getBool(kWeatherUnitsMetricNotImperialKey);
  preferences.end();
  PrintLn(__func__, weather_units_metric_not_imperial);
  return weather_units_metric_not_imperial;
}

void NvsPreferences::SaveWeatherUnits(bool weather_units_metric_not_imperial) {
  preferences.begin(kNvsDataKey, /*readOnly = */ false);
  preferences.putBool(kWeatherUnitsMetricNotImperialKey, weather_units_metric_not_imperial);
  preferences.end();
  PrintLn(__func__, (weather_units_metric_not_imperial ? kMetricUnitStr : kImperialUnitStr));
}

uint8_t NvsPreferences::RetrieveSavedCpuSpeed() {
  preferences.begin(kNvsDataKey, /*readOnly = */ true);
  uint8_t saved_cpu_speed_mhz = preferences.getUChar(kCpuSpeedMhzKey);
  preferences.end();
  if(saved_cpu_speed_mhz < 80)
    saved_cpu_speed_mhz = 80;
  PrintLn(__func__, saved_cpu_speed_mhz);
  return saved_cpu_speed_mhz;
}

void NvsPreferences::SaveCpuSpeed() {
  preferences.begin(kNvsDataKey, /*readOnly = */ false);
  preferences.putUChar(kCpuSpeedMhzKey, cpu_speed_mhz);
  preferences.end();
  PrintLn(__func__, cpu_speed_mhz);
}

bool NvsPreferences::RetrieveScreensaverBounceNotFlyHorizontally() {
  preferences.begin(kNvsDataKey, /*readOnly = */ true);
  bool screensaver_bounce_not_fly_horiontally = preferences.getBool(kScreensaverMotionTypeKey);
  preferences.end();
  PrintLn(__func__, screensaver_bounce_not_fly_horiontally);
  return screensaver_bounce_not_fly_horiontally;
}

void NvsPreferences::SaveScreensaverBounceNotFlyHorizontally(bool screensaver_bounce_not_fly_horiontally) {
  preferences.begin(kNvsDataKey, /*readOnly = */ false);
  preferences.putBool(kScreensaverMotionTypeKey, screensaver_bounce_not_fly_horiontally);
  preferences.end();
  PrintLn(__func__, screensaver_bounce_not_fly_horiontally);
}

bool NvsPreferences::RetrieveScreensaverSleepFriendNightColor() {
  preferences.begin(kNvsDataKey, /*readOnly = */ true);
  bool sleep_friendly_color_at_night = preferences.getBool(kScreensaverSleepFriendlyColorAtNightKey);
  preferences.end();
  PrintLn(__func__, sleep_friendly_color_at_night);
  return sleep_friendly_color_at_night;
}

void NvsPreferences::SaveScreensaverSleepFriendNightColor(bool sleep_friendly_color_at_night) {
  preferences.begin(kNvsDataKey, /*readOnly = */ false);
  preferences.putBool(kScreensaverSleepFriendlyColorAtNightKey, sleep_friendly_color_at_night);
  preferences.end();
  PrintLn(__func__, sleep_friendly_color_at_night);
}

uint8_t NvsPreferences::RetrieveNightTimeDimHour() {
  preferences.begin(kNvsDataKey, /*readOnly = */ true);
  uint8_t night_time_dim_hour = preferences.getUChar(kNightTimeDimHourKey);
  preferences.end();
  PrintLn(__func__, night_time_dim_hour);
  return night_time_dim_hour;
}

void NvsPreferences::SaveNightTimeDimHour(uint8_t night_time_dim_hour) {
  preferences.begin(kNvsDataKey, /*readOnly = */ false);
  preferences.putUChar(kNightTimeDimHourKey, night_time_dim_hour);
  preferences.end();
  PrintLn(__func__, night_time_dim_hour);
}

uint8_t NvsPreferences::RetrieveScreenOrientation() {
  preferences.begin(kNvsDataKey, /*readOnly = */ true);
  uint8_t screen_orientation = preferences.getUChar(kScreenOrientationKey);
  preferences.end();
  PrintLn(__func__, screen_orientation);
  return screen_orientation;
}

void NvsPreferences::SaveScreenOrientation(uint8_t screen_orientation) {
  preferences.begin(kNvsDataKey, /*readOnly = */ false);
  preferences.putUChar(kScreenOrientationKey, screen_orientation);
  preferences.end();
  PrintLn(__func__, screen_orientation);
}

uint8_t NvsPreferences::RetrieveAutorunRgbLedStripMode() {
  preferences.begin(kNvsDataKey, /*readOnly = */ true);
  uint8_t autorun_rgb_led_strip_mode_retrieved = preferences.getUChar(kAutorunRgbLedStripModeKey, 0);
  preferences.end();
  PrintLn(__func__, autorun_rgb_led_strip_mode_retrieved);
  return autorun_rgb_led_strip_mode_retrieved;
}

void NvsPreferences::SaveAutorunRgbLedStripMode(uint8_t autorun_rgb_led_strip_mode_to_save) {
  preferences.begin(kNvsDataKey, /*readOnly = */ false);
  preferences.putUChar(kAutorunRgbLedStripModeKey, autorun_rgb_led_strip_mode_to_save);
  preferences.end();
  PrintLn(__func__, autorun_rgb_led_strip_mode_to_save);
}

bool NvsPreferences::RetrieveUseLdr() {
  preferences.begin(kNvsDataKey, /*readOnly = */ true);
  bool use_ldr = preferences.getBool(kUseLDRKey);
  preferences.end();
  PrintLn(__func__, use_ldr);
  return use_ldr;
}

void NvsPreferences::SaveUseLdr(bool use_ldr) {
  preferences.begin(kNvsDataKey, /*readOnly = */ false);
  preferences.putBool(kUseLDRKey, use_ldr);
  preferences.end();
  PrintLn(__func__, use_ldr);
}

uint8_t NvsPreferences::RetrieveTouchscreenType() {
  preferences.begin(kNvsDataKey, /*readOnly = */ true);
  uint8_t touchscreen_type = preferences.getUChar(kTouchscreenTypeKey);
  preferences.end();
  PrintLn(__func__, touchscreen_type);
  return touchscreen_type;
}

void NvsPreferences::SaveTouchscreenType(uint8_t touchscreen_type) {
  preferences.begin(kNvsDataKey, /*readOnly = */ false);
  preferences.putUChar(kTouchscreenTypeKey, touchscreen_type);
  preferences.end();
  PrintLn(__func__, touchscreen_type);
}

bool NvsPreferences::RetrieveTouchscreenFlip() {
  preferences.begin(kNvsDataKey, /*readOnly = */ true);
  bool touchscreen_flip = preferences.getBool(kTouchscreenFlipKey);
  preferences.end();
  PrintLn(__func__, touchscreen_flip);
  return touchscreen_flip;
}

void NvsPreferences::SaveTouchscreenFlip(bool touchscreen_flip) {
  preferences.begin(kNvsDataKey, /*readOnly = */ false);
  preferences.putBool(kTouchscreenFlipKey, touchscreen_flip);
  preferences.end();
  PrintLn(__func__, touchscreen_flip);
}

uint8_t NvsPreferences::RetrieveRgbStripLedCount() {
  preferences.begin(kNvsDataKey, /*readOnly = */ true);
  uint8_t rgb_strip_led_count = preferences.getUChar(kRgbStripLedCountKey, 0);
  preferences.end();
  PrintLn(__func__, rgb_strip_led_count);
  return rgb_strip_led_count;
}

void NvsPreferences::SaveRgbStripLedCount(uint8_t rgb_strip_led_count) {
  preferences.begin(kNvsDataKey, /*readOnly = */ false);
  preferences.putUChar(kRgbStripLedCountKey, rgb_strip_led_count);
  preferences.end();
  PrintLn(__func__, rgb_strip_led_count);
}

uint8_t NvsPreferences::RetrieveRgbStripLedBrightness() {
  preferences.begin(kNvsDataKey, /*readOnly = */ true);
  uint8_t rgb_strip_led_brightness = preferences.getUChar(kRgbStripLedBrightnessKey, 0);
  preferences.end();
  PrintLn(__func__, rgb_strip_led_brightness);
  return rgb_strip_led_brightness;
}

void NvsPreferences::SaveRgbStripLedBrightness(uint8_t rgb_strip_led_brightness) {
  preferences.begin(kNvsDataKey, /*readOnly = */ false);
  preferences.putUChar(kRgbStripLedBrightnessKey, rgb_strip_led_brightness);
  preferences.end();
  PrintLn(__func__, rgb_strip_led_brightness);
}

uint8_t NvsPreferences::RetrieveRtcType() {
  preferences.begin(kNvsDataKey, /*readOnly = */ true);
  uint8_t rtc_type = preferences.getUChar(kRtcTypeKey, 0);
  preferences.end();
  std::string rtc_str = "";
  switch(rtc_type) {
    case 1:
      rtc_str = "DS1307";
      break;
    case 2:
      rtc_str = "DS3231";
      break;
    default:
      rtc_str = "Unknown";
  }
  PrintLn(__func__, (std::to_string(rtc_type) + ":" + rtc_str));
  return rtc_type;
}

void NvsPreferences::SaveRtcType(uint8_t rtc_type) {
  preferences.begin(kNvsDataKey, /*readOnly = */ false);
  preferences.putUChar(kRtcTypeKey, rtc_type);
  preferences.end();
  PrintLn(__func__, rtc_type);
}

void NvsPreferences::RetrieveOwnerName(std::string &owner_name) {
  preferences.begin(kNvsDataKey, /*readOnly = */ true);
  String owner_name_string = preferences.getString(kOwnerNameKey);
  preferences.end();
  owner_name = owner_name_string.c_str();
  PrintLn(__func__, owner_name);
}

void NvsPreferences::SaveOwnerName(std::string owner_name) {
  preferences.begin(kNvsDataKey, /*readOnly = */ false);
  if(owner_name.length() == 0)
    owner_name = kOwnerName;
  String owner_name_string = owner_name.c_str();
  preferences.putString(kOwnerNameKey, owner_name_string);
  preferences.end();
  PrintLn(__func__, owner_name);
}

void NvsPreferences::RemoveKey(std::string remove_key) {
  preferences.begin(kNvsDataKey, /*readOnly = */ false);
  if(preferences.isKey(remove_key.c_str())) {
    preferences.remove(remove_key.c_str());
    PrintLn("Removed key =", remove_key);
  }
  else {
    PrintLn("Key not found =", remove_key);
  }
  preferences.end();
}

void NvsPreferences::RetrieveTouchScreenCalibration(int16_t &xMin, int16_t &xMax, int16_t &yMin, int16_t &yMax) {
  preferences.begin(kNvsDataKey, /*readOnly = */ true);
  xMin = preferences.getShort(kTouchCalibXMinKey, kTouchCalibXMin);
  xMax = preferences.getShort(kTouchCalibXMaxKey, kTouchCalibXMax);
  yMin = preferences.getShort(kTouchCalibYMinKey, kTouchCalibYMin);
  yMax = preferences.getShort(kTouchCalibYMaxKey, kTouchCalibYMax);
  preferences.end();
  PrintLn(__func__, ("xMin=" + std::to_string(xMin) + ", xMax=" + std::to_string(xMax) + ", yMin=" + std::to_string(yMin) + ", yMax=" + std::to_string(yMax)));
}

void NvsPreferences::SaveTouchScreenCalibration(int16_t xMin, int16_t xMax, int16_t yMin, int16_t yMax) {
  preferences.begin(kNvsDataKey, /*readOnly = */ false);
  preferences.putShort(kTouchCalibXMinKey, xMin);
  preferences.putShort(kTouchCalibXMaxKey, xMax);
  preferences.putShort(kTouchCalibYMinKey, yMin);
  preferences.putShort(kTouchCalibYMaxKey, yMax);
  preferences.end();
  PrintLn(__func__, ("xMin=" + std::to_string(xMin) + ", xMax=" + std::to_string(xMax) + ", yMin=" + std::to_string(yMin) + ", yMax=" + std::to_string(yMax)));
}

uint8_t NvsPreferences::RetrieveHwVersion() {
  preferences.begin(kNvsDataKey, /*readOnly = */ true);
  uint8_t hw_version = preferences.getUChar(kHwVersionKey, 0);
  preferences.end();
  PrintLn(__func__, hw_version);
  return hw_version;
}

void NvsPreferences::RetrieveCityName(std::string &city_name) {
  preferences.begin(kNvsDataKey, /*readOnly = */ true);
  String city_name_string = preferences.getString(kCityNameKey);
  city_name = city_name_string.c_str();
  preferences.end();
  PrintLn(__func__, city_name);
}

void NvsPreferences::SaveCityName(std::string city_name) {
  preferences.begin(kNvsDataKey, /*readOnly = */ false);
  String city_name_string = city_name.c_str();
  preferences.putString(kCityNameKey, city_name_string);
  preferences.end();
  PrintLn(__func__, city_name);
}

uint8_t NvsPreferences::RetrieveDisplayMinBrightnessAdder() {
  preferences.begin(kNvsDataKey, /*readOnly = */ true);
  uint8_t display_min_brightness_adder = preferences.getUChar(kDisplayMinBrightnessAdderKey, 0);
  preferences.end();
  PrintLn(__func__, display_min_brightness_adder);
  return display_min_brightness_adder;
}

void NvsPreferences::SaveDisplayMinBrightnessAdder(uint8_t display_min_brightness_adder) {
  preferences.begin(kNvsDataKey, /*readOnly = */ false);
  preferences.putUChar(kDisplayMinBrightnessAdderKey, display_min_brightness_adder);
  preferences.end();
  PrintLn(__func__, display_min_brightness_adder);
}

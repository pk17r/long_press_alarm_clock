#include "lwipopts.h"
#include "uRTCLib.h"
#include "rtc.h"
#include "nvs_preferences.h"

// RTC constructor
RTC::RTC() {

  /* INITIALIZE RTC */

  // initialize Wire lib
  // URTCLIB_WIRE.setPins(SDA_PIN(), SCL_PIN());
  // URTCLIB_WIRE = Wire;
  // Wire.setPins(SDA_PIN(), SCL_PIN());
  URTCLIB_WIRE.begin(SDA_PIN(), SCL_PIN());

  URTCLIB_WIRE.beginTransmission(URTCLIB_ADDRESS);
  byte response = URTCLIB_WIRE.endTransmission();

  char rtc_addr_hex_char_arr[5];
  sprintf(rtc_addr_hex_char_arr, "0x%02X", URTCLIB_ADDRESS);
  std::string print_str = "I2C RTC at " + std::string(rtc_addr_hex_char_arr) + std::string(response == 0 ? " found" : " NOT FOUND");
  PrintLn(print_str);

  // setup DS1307/DS3231 rtc
  RtcSetup();

  PrintLn(__func__, kInitializedStr);
}

// setup DS1307/DS3231 rtc
void RTC::RtcSetup() {
  // set rtc model
  rtc_hw_.set_model(nvs_preferences->RetrieveRtcType());    // 1 = URTCLIB_MODEL_DS1307, 2 = URTCLIB_MODEL_DS3231

  // get data from DS1307/DS3231 HW
  Refresh();

  // Set Oscillator to use VBAT when VCC turns off if not set
  if(rtc_hw_.getEOSCFlag()) {

    #ifdef MORE_LOGS
    if(rtc_hw_.enableBattery())
      PrintLn("Enable Battery Success");
    else
      PrintLn("Enable Battery UNSUCCESSFUL!");
    #else
    rtc_hw_.enableBattery();
    #endif

    delay(100);
  }

  // disable 32K Pin Sq Wave out if on
  if(rtc_hw_.status32KOut()) {
    rtc_hw_.disable32KOut();
    #ifdef MORE_LOGS
    PrintLn("disable32KOut() done");
    #endif
    delay(100);
  }

  // set sqw pin to trigger every second
  rtc_hw_.sqwgSetMode(URTCLIB_SQWG_1H);
  delay(100);

  // clear alarms flags if any
  if(rtc_hw_.alarmTriggered(URTCLIB_ALARM_1)) {
    rtc_hw_.alarmClearFlag(URTCLIB_ALARM_1);
    #ifdef MORE_LOGS
    PrintLn("URTCLIB_ALARM_1 alarm flag cleared.");
    #endif
    delay(100);
  }
  if(rtc_hw_.alarmTriggered(URTCLIB_ALARM_2)) {
    rtc_hw_.alarmClearFlag(URTCLIB_ALARM_2);
    #ifdef MORE_LOGS
    PrintLn("URTCLIB_ALARM_2 alarm flag cleared.");
    #endif
    delay(100);
  }

  // we won't use RTC for alarm, disable if enabled
  if(rtc_hw_.alarmMode(URTCLIB_ALARM_1) != URTCLIB_ALARM_TYPE_1_NONE) {
    rtc_hw_.alarmDisable(URTCLIB_ALARM_1);
    #ifdef MORE_LOGS
    PrintLn("URTCLIB_ALARM_1 disabled.");
    #endif
    delay(100);
  }
  if(rtc_hw_.alarmMode(URTCLIB_ALARM_2) != URTCLIB_ALARM_TYPE_2_NONE) {
    rtc_hw_.alarmDisable(URTCLIB_ALARM_2);
    #ifdef MORE_LOGS
    PrintLn("URTCLIB_ALARM_2 disabled.");
    #endif
    delay(100);
  }

  // set rtcHw in 12 hour mode if not already
  if(rtc_hw_.hourModeAndAmPm() == 0) {
    rtc_hw_.set_12hour_mode(true);
    delay(100);
  }

  #ifdef MORE_LOGS
  uint8_t hr_mode = rtc_hw_.hourModeAndAmPm();
  std::string hr_mode_str = "RTC Hour Mode = ";
  if(hr_mode == 0)
    hr_mode_str += "24-hr mode";
  else if(hr_mode == 1)
    hr_mode_str += "12-hr mode AM";
  else
    hr_mode_str += "12-hr mode PM";
  PrintLn(hr_mode_str);
  #endif


  // Check if time is up to date
  // Lost power status:
  if (rtc_hw_.lostPower()) {
    #ifdef MORE_LOGS
    PrintLn("POWER FAILED. Clearing flag...");
    #endif
    rtc_hw_.lostPowerClear();
    delay(100);
  }
  #ifdef MORE_LOGS
  else {
    PrintLn("POWER OK");
  }
  #endif

  #ifdef MORE_LOGS
  // Check whether OSC is set to use VBAT or not
  if (rtc_hw_.getEOSCFlag())
    PrintLn("Oscillator will NOT use VBAT when VCC cuts off. Time will not increment without VCC!");
  else
    PrintLn("Oscillator will use VBAT if VCC cuts off.");
  #endif

  #ifdef MORE_LOGS
  std::string rtc_time_str = std::to_string(hour()) + ":" + (minute() < 10 ? "0" : "") + std::to_string(minute()) + ":" + (second() < 10 ? "0" : "") + std::to_string(second()) + (hourModeAndAmPm() == 1 ? kAmLabel : (hourModeAndAmPm() == 2 ? kPmLabel : "--")) + ", dayOfWeek(1=Sun)=" + std::to_string(dayOfWeek()) + ", " + std::to_string(month()) + "/" + std::to_string(day()) + "/" + std::to_string(year());
  PrintLn("RTC Time:", rtc_time_str);
  #endif

  // set seconds interrupt
  SetSecondsInterrupt(/*set = */ true);
}

void RTC::SetSecondsInterrupt(bool set) {
  // seconds interrupt pin
  pinMode(SQW_INT_PIN, INPUT_PULLUP);
  if(set)
    attachInterrupt(digitalPinToInterrupt(SQW_INT_PIN), SecondsUpdateInterruptISR, RISING);
  else
    detachInterrupt(digitalPinToInterrupt(SQW_INT_PIN));
}

// private function to refresh time from RTC HW and do basic power failure checks
void RTC::Refresh() {

  // refresh time in class object from RTC HW
  rtc_hw_.refresh();
  rtc_refresh_reqd_ = false;

  // make _second equal to rtcHw seconds -> should be 0 at minute updates
  second_ = rtc_hw_.second();

  // PrintLn("__RTC Refresh__ ");

  SetTodaysMinutes();

  #ifdef MORE_LOGS
  // Check whether RTC HW experienced a power loss and thereby know if time is up to date or not
  if (rtc_hw_.lostPower())
    PrintLn(__func__, "lostPower");
  // Check whether RTC HW Oscillator is set to use VBAT or not
  if(rtc_hw_.getEOSCFlag())
    PrintLn(__func__, "getEOSCFlag");   // Oscillator will not use VBAT when VCC cuts off. Time will not increment without VCC!
  #endif

}

// clock seconds interrupt ISR
void IRAM_ATTR RTC::SecondsUpdateInterruptISR() {
  // update seconds
  second_++;
  // a flag for others that time has updated!
  rtc_hw_sec_update_ = true;

  // refresh time on rtc class object on new minute
  if(second_ >= 60) {
    rtc_hw_min_update_ = true;
    rtc_refresh_reqd_ = true;
  }
}

uint8_t RTC::minute() {
  if(rtc_refresh_reqd_)
    Refresh();
  return rtc_hw_.minute();
}

uint8_t RTC::hour() {
  if(rtc_refresh_reqd_)
    Refresh();
  return rtc_hw_.hour();
}

/**
 * \brief Sets RTC HW datetime data with input Hr in 24 hour mode and puts RTC to 12 hour mode
 *
 * @param second second
 * @param minute minute
 * @param hour_24_hr_mode hour to set in 24 hour mode
 * @param dayOfWeek_Sun_is_1 day of week with Sunday = 1
 * @param day today's date
 * @param month_Jan_is_1 month with January = 1
 * @param year year
 */
bool RTC::SetRtcTimeAndDate(uint8_t second, uint8_t minute, uint8_t hour_24_hr_mode, uint8_t dayOfWeek_Sun_is_1, uint8_t day, uint8_t month_Jan_is_1, uint16_t year) {
  // unset seconds interrupt
  SetSecondsInterrupt(/*set = */ false);

  // Set current time and date and 24 hour mode on RTC
  // RTCLib::set(byte second, byte minute, byte hour, byte dayOfWeek, byte dayOfMonth, byte month, byte year)
  rtc_hw_.set(second, minute, hour_24_hr_mode, dayOfWeek_Sun_is_1, day, month_Jan_is_1, year - 2000);
  // refresh time from RTC HW
  Refresh();
  // set RTC HW back into 12 hour mode
  rtc_hw_.set_12hour_mode(true);
  // refresh time from RTC HW
  Refresh();

  uint8_t hour_12_hr_mode = 0;
  if(hour_24_hr_mode == 0)
    hour_12_hr_mode = 12;
  else if(hour_24_hr_mode <= 12)
    hour_12_hr_mode = hour_24_hr_mode;
  else
    hour_12_hr_mode = hour_24_hr_mode - 12;

  // check correct updation of time
  bool update_success = true;
  uint8_t hr_updated = rtc_hw_.hour();
  uint8_t min_updated = rtc_hw_.minute();
  uint8_t sec_updated = rtc_hw_.second();
  uint8_t doW_updated = rtc_hw_.dayOfWeek();
  uint8_t day_updated = rtc_hw_.day();
  uint8_t month_updated = rtc_hw_.month();
  uint16_t yr_updated = rtc_hw_.year() + 2000;
  if((hr_updated != hour_12_hr_mode) || (min_updated != minute) || (doW_updated != dayOfWeek_Sun_is_1) || (day_updated != day) || (month_updated != month_Jan_is_1) || (yr_updated != year))
    update_success = false;

  // re-set seconds interrupt
  SetSecondsInterrupt(/*set = */ true);

  #ifdef MORE_LOGS
  if(!update_success) {
    PrintLn("Time Update Entered:");
    PrintLn("hour_24_hr_mode: ", hour_24_hr_mode);
    PrintLn("hour_12_hr_mode: ", hour_12_hr_mode);
    PrintLn("minute: ", minute);
    PrintLn("second: ", second);
    PrintLn("dayOfWeek_Sun_is_1: ", dayOfWeek_Sun_is_1);
    PrintLn("day: ", day);
    PrintLn("month_Jan_is_1: ", month_Jan_is_1);
    PrintLn("year: ", year);

    PrintLn("Time Update After Update:");
    PrintLn("hr_updated: ", hr_updated);
    PrintLn("min_updated: ", min_updated);
    PrintLn("sec_updated: ", sec_updated);
    PrintLn("doW_updated: ", doW_updated);
    PrintLn("day_updated: ", day_updated);
    PrintLn("month_updated: ", month_updated);
    PrintLn("yr_updated: ", yr_updated);
  }
  #endif
  PrintLn(__func__, (std::string("update_success=") + std::string(update_success ? "true":"false")));
  return update_success;
}

void RTC::SetTodaysMinutes() {
  uint16_t todays_minutes_temp = minute();
  if(hourModeAndAmPm() == 0) {
    // 24 hour mode
    todays_minutes_temp += hour() * 60;
  }
  else {
    // 12 hour mode
    if(hour() != 12)
      todays_minutes_temp += hour() * 60;
    if(hourModeAndAmPm() == 2)
      todays_minutes_temp += 12 * 60;
  }
  todays_minutes = todays_minutes_temp;
}

  /**
  * \brief Converts Minute of the day into HH:MM XM (Only for 12 hour mode)
  * todays_minutes_val = Minute of the day 0-1439 minutes
  * @return 
  * hour_mode_and_am_pm_flag = 1 = 12 hour mode AM hours (1-12 hours)
  *                       2 = 12 hour mode PM hours (1-12 hours)
  * hr = Hour
  * min = Minute
  */
void RTC::MinutesToHHMMXM(uint16_t todays_minutes_val, uint8_t &hour_mode_and_am_pm_flag, uint8_t &hr, uint8_t &min) {
  if(todays_minutes_val >= 60 * 12) {
    hour_mode_and_am_pm_flag = 2;
    todays_minutes_val -= 60 * 12;
  }
  else
    hour_mode_and_am_pm_flag = 1;

  min = todays_minutes_val % 60;

  uint8_t hr_temp = todays_minutes_val / 60;

  if(hr_temp == 0)
    hr = 12;
  else
    hr = hr_temp;
}

  /**
  * \brief Converts HH:MM XM into Minute of the day
  * hour_mode_and_am_pm_flag = 0 = 24 hour mode (0-23 hours)
  *                       1 = 12 hour mode AM hours (1-12 hours)
  *                       2 = 12 hour mode PM hours (1-12 hours)
  * hr = Hour
  * min = Minute
  * @return Minute of the day 0-1439 minutes
  */
uint16_t RTC::HHMMXMToMinutes(uint8_t hour_mode_and_am_pm_flag, uint8_t hr, uint8_t min) {
  uint16_t todays_minutes_temp = min;
  if(hour_mode_and_am_pm_flag == 0) {
    // 24 hour mode
    todays_minutes_temp += hr * 60;
  }
  else {
    // 12 hour mode
    if(hr != 12)
      todays_minutes_temp += hr * 60;
    if(hour_mode_and_am_pm_flag == 2)
      todays_minutes_temp += 12 * 60;
  }
  return todays_minutes_temp;
}

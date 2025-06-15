/**************************************************************************

# Long Press Alarm Clock

When alarm time hits, program requires user to press and hold the main LED push button for 20 (adjustable) seconds continously to turn off the alarm, making sure the user wakes up.
Without the button press, buzzer keeps buzzing. If user lets go of the push button before alarm end time, the buzzer restarts. Max buzzer time of 120 seconds.


Github: https://github.com/pk17r/Long_Press_Alarm_Clock/tree/release


- Software:
  - A fast very low RAM usage FastDrawTwoColorBitmapSpi display function is implemented that converts a full or section of a monochrome frame into 16-bit RGB565 frame with 2 colors 
  and sends data to display via SPI row by row in under 50ms for a full 320x240px frame and in only 20ms for 40% sized frame. This achieves a FPS of 20 frames per second for a
  full frame and a whooping 50 frames per second for 40% sized frames. Using this way of converting a monochrome image into 2 colors row by row saves a lot of RAM on the MCU as now
  we don't have to populate the whole 16-bit RGB565 frame, but only a 1-bit monochrome frame. This way a 153kB RGB565 frame on a 320x240px display is reduced to just 9.6kB, allowing 
  usage of lower RAM MCUs and much faster processing times per frame. A 40% sized canvas of a 320x240px display is made within 7ms on a 240MHz esp32. The screensaver implemented on
  this device achieves a whooping 45-50 frames per second speeds.
  - C++ OOP Based Project
  - All modules fully distributed in independent classes and header files
  - Arduino setup and loop functions in .ino file
  - MCU Selection and Module selections in configuration.h file, pin definitions in pin_defs.h file
  - A common header containing pointers to objects of every module and global functions
  - Adafruit Library used for GFX functions
  - uRTCLib Library for DS1307/DS3231 updated with AM/PM mode and class size reduced by 3 bytes while adding additional functionality
  - Secure Web Over The Air Firmware Update Functionality
  - Watchdog keeps a check on the program and reboots MCU if it gets stuck
  - Modular programming that fits single core or dual core microcontrollers


- Hardware:
  - Microcontroller: ESP32 S3 (Default) or ESP32 S2 Mini or ESP32 WROOM
  - Display: 2.8" ST7789V display (Default), other selectable options: ST7735, ILI9341 and ILI9488
  - Touchscreen XPT2046 (not enabled by default)
  - DS1307/DS3231 RTC Clock IC
  - A push button with LED
  - 2 push buttons for increase and decrease functions
  - An 85dB passive buzzer for alarm and different frequency tones


- Salient Features
  - There is no alarm snooze button.
  - Time update via NTP server using WiFi once every day to maintain high accuracy
  - DS1307/DS3231 RTC itself is high accuracy clock having deviation of +/-2 minutes per year
  - Time auto adjusts for time zone and day light savings with location ZIP/PIN and country code
  - Get Weather info using WiFi and display today's weather after alarm
  - Get user input of WiFi details via an on-screen keyboard (when touchscreen is used and enabled)
  - Colorful Smooth Screensaver with a big clock
  - Touchscreen based alarm set page (touchscreen not on by default)
  - Settings saved in ESP32 NVM so not lost on power loss
  - Screen brightness changes according to time of the day, with lowest brightness setting at night time
  - Time critical tasks happen on core0 - time update, screensaver fast motion, alarm time trigger
  - Non Time critical tasks happen on core1 - update weather info using WiFi, update time using NTP server, connect/disconnect WiFi
  - Very Low Power usage of 0.5W during day and 0.3W during night time


- Datasheets:
  - ESP32 Lolin S2 Mini Single Core MCU Board https://www.wemos.cc/en/latest/s2/s2_mini.html
  - ESP32 Lolin S2 Mini Pinouts https://www.studiopieters.nl/wp-content/uploads/2022/08/WEMOS-ESP32-S2-MINI.png
  - 2.8" Touchscreen ST7789V driver https://www.aliexpress.us/item/3256805747165796.html
  - 2.8" Touchscreen ILI9341 driver http://www.lcdwiki.com/2.8inch_SPI_Module_ILI9341_SKU:MSP2807


Prashant Kumar


***************************************************************************/
#include "common.h"
#include <PushButtonTaps.h>
#include "nvs_preferences.h"
#if defined(WIFI_IS_USED)
  #include "wifi_stuff.h"
#endif
#include "rtc.h"
#include "alarm_clock.h"
#include "rgb_display.h"
#include "touchscreen.h"
#include <Adafruit_I2CDevice.h>
#include <Adafruit_NeoPixel.h>

// modules - hardware or software
PushButtonTaps* push_button = NULL;   // Push Button object
PushButtonTaps* inc_button = NULL;   // Push Button object
PushButtonTaps* dec_button = NULL;   // Push Button object
NvsPreferences* nvs_preferences = NULL;    // ptr to NVS Preferences class object
WiFiStuff* wifi_stuff = NULL;  // ptr to wifi stuff class object that contains WiFi and Weather Fetch functions
RTC* rtc = NULL;  // ptr to class object containing RTC HW
AlarmClock* alarm_clock = NULL;  // ptr to alarm clock class object that controls Alarm functions
RGBDisplay* display = NULL;   // ptr to display class object that manages the display
Touchscreen* ts = NULL;         // Touchscreen class object

// LOCAL PROGRAM VARIABLES

#if defined(ESP32_DUAL_CORE)
  TaskHandle_t Task1;
#endif
// watchdog
#if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
  // Code for version 3.x
  #include "esp_system.h"
  #include "rom/ets_sys.h"
  hw_timer_t *watchdog_timer = NULL;

  void ARDUINO_ISR_ATTR resetModule() {
    ets_printf("reboot\n");
    esp_restart();
  }
#else
  // Code for version 2.x
  #include <esp_task_wdt.h>   // ESP32 Watchdog header
#endif

// Arduino SPI Class Object
SPIClass* spi_obj = NULL;

// random afternoon hour and minute to update firmware
uint16_t ota_update_minute = 0;

// RGB LED Strip Neopixels
Adafruit_NeoPixel* rgb_led_strip = NULL;
int rgb_strip_led_count = 4;  // rgb_led_strip
bool rgb_led_strip_on = false;
uint16_t current_led_strip_color = 0x6D9D;    // RGB565_Argentinian_blue
const uint32_t kDefaultLedStripColor = 0xFFFFFF;       // White
uint8_t rgb_strip_led_brightness = 255;
#ifdef MORE_LOGS
uint8_t frames_per_second = 0;
#endif

enum ButtonClickUiFeedbackType {
  kTurnOn_Delay_TurnOff,
  kTurnOn_Delay,
  kTurnOn,
  kTurnOff
};

// LOCAL FUNCTIONS
// populate all pages in display_pages_vec
void PopulateDisplayPages();
int DisplayPagesVecCurrentButtonIndex();
int DisplayPagesVecButtonIndex(ScreenPage button_page, Cursor button_cursor);
void ButtonClickUiFeedback(ButtonClickUiFeedbackType response_type);
void InitializeRgbLed();
void RunRgbLedAccordingToSettings();
const char* RgbLedSettingString();
void WiFiPasswordInputTouchAndNonTouch();

// setup core1
void setup() {

  // a delay to let currents stabalize and not have phantom serial inputs
  delay(200);
  Serial.begin(115200);
  // while (!Serial) {
  //   delay(500);
  // }
  Serial.println(F("\nSerial OK"));
  PrintLn("Hellow World!");

  // Firmware version and date on program
  PrintLn("kFirmwareVersion:", kFirmwareVersion);
  PrintLn("kFirmwareDate:", kFirmwareDate);

  // setup nvs preferences data (needs to be first)
  nvs_preferences = new NvsPreferences();
  PrintLn("My_Hw_Version = ", My_Hw_Version);
  // check if firmware was updated
  std::string saved_firmware_version = nvs_preferences->RetrieveSavedFirmwareVersion();
  if(strcmp(saved_firmware_version.c_str(), kFirmwareVersion.c_str()) != 0) {
    firmware_updated_flag_user_information = true;
    nvs_preferences->SaveCurrentFirmwareVersion();
  }

  // LED Pin High
  pinMode(LED_PIN(), OUTPUT);
  LedFeedback(HIGH);

  // make buzzer pin low
  pinMode(BUZZER_PIN(), OUTPUT);
  digitalWrite(BUZZER_PIN(), LOW);

  // display spi CS pin high
  pinMode(TFT_CS, OUTPUT);
  digitalWrite(TFT_CS, HIGH);

  // TFT_RST - pull it low
  pinMode(TFT_RST, OUTPUT);
  digitalWrite(TFT_RST, LOW);

  // pullup debug pin
  pinMode(DEBUG_PIN, INPUT_PULLUP);

  if(0x01 == My_Hw_Version) {
    // WIFI_LED Low
    pinMode(WIFI_LED(), OUTPUT);
    digitalWrite(WIFI_LED(), LOW);

    // XRP2046 spi CS pin high
    pinMode(TS_CS_PIN, OUTPUT);
    digitalWrite(TS_CS_PIN, HIGH);
  }

  // check if in debug mode
  debug_mode = !digitalRead(DEBUG_PIN);
  // debug_mode = true;
  if(debug_mode) {
    delay(2000);
    // while(!Serial) { delay(20); };   // Do not uncomment during commit!
    Serial.println(F("\n\n******** DEBUG MODE ******** : watchdog won't be activated!"));
  }
  else {
    // enable watchdog reset if not in debug mode
    SetWatchdogTime(kWatchdogTimeoutMs);
  }
  Serial.flush();

  // initialize hardware spi
  spi_obj = new SPIClass(HSPI);
  spi_obj->begin(TFT_CLK, TS_CIPO, TFT_COPI, TFT_CS); //SCLK, MISO, MOSI, SS

  // initialize push button
  push_button = new PushButtonTaps(BUTTON_PIN());
  if(My_Hw_Version == 0x01) {
    inc_button = new PushButtonTaps(INC_BUTTON_PIN());
    dec_button = new PushButtonTaps(DEC_BUTTON_PIN());
  }

  // initialize modules
  // setup rtc (needs to be before alarm clock)
  rtc = new RTC();
  // setup alarm clock (needs to be before display)
  alarm_clock = new AlarmClock();
  alarm_clock->Setup();
  // prepare date and time arrays and serial print RTC Date Time
  PrepareTimeDayDateArrays();
  // serial print RTC Date Time
  SerialPrintRtcDateTime();
  // initialize wifi (needs to be before display setup)
  #if defined(WIFI_IS_USED)
    wifi_stuff = new WiFiStuff();
  #endif
  // check if hardware has LDR
  use_photoresistor = nvs_preferences->RetrieveUseLdr();
  // initialize display class object
  display = new RGBDisplay();
  // setup and populate display
  display->Setup();
  // touchscreen type
  if(nvs_preferences->RetrieveTouchscreenType())
    ts = new Touchscreen();

  // second core task added flag array
  for (int i = 0; i < kNoTask; i++)
    second_core_task_added_flag_array[i] = false;

  // pick random time between 10AM and 6PM for firmware OTA update
  {
    // initialize random seed
    unsigned long seed = rtc->minute() * 60 + rtc->second();
    randomSeed(seed);
    // ota update minute
    ota_update_minute = random(600, 1080);
    uint8_t ota_update_hr, ota_update_min, ota_update_am_pm_flag;
    rtc->MinutesToHHMMXM(ota_update_minute, ota_update_am_pm_flag, ota_update_hr, ota_update_min);
    PrintLn("OTA Update at", (std::to_string(ota_update_hr) + ":" + std::to_string(ota_update_min) + " " + (ota_update_am_pm_flag == 1 ? kAmLabel : kPmLabel)));
  }

  // set CPU Speed
  setCpuFrequencyMhz(nvs_preferences->RetrieveSavedCpuSpeed());
  cpu_speed_mhz = getCpuFrequencyMhz();
  nvs_preferences->SaveCpuSpeed();

  // initialize rgb led strip neopixels
  InitializeRgbLed();
  RunRgbLedAccordingToSettings();

  PopulateDisplayPages(); // needs to be after all saved values have been retrieved

  #if defined(ESP32_DUAL_CORE)
    xTaskCreatePinnedToCore(
        Task1code, /* Function to implement the task */
        "Task1", /* Name of the task */
        10000,  /* Stack size in words */
        NULL,  /* Task input parameter */
        0,  /* Priority of the task */
        &Task1,  /* Task handle. */
        0); /* Core where the task should run */
  #endif

  ResetWatchdog();
  SerialInputFlush();

  //////////////////////////////////////////////////

  // House-cleaning zone

  // rename old default owner name
  {
    const std::string kOwnerName_old1 = "?";
    const std::string kOwnerName_old2 = "<name>";
    const std::string kOwnerName_new = "Adventurer";
    std::string OwnerName_current;
    nvs_preferences->RetrieveOwnerName(OwnerName_current);
    if((OwnerName_current == kOwnerName_old1) || (OwnerName_current == kOwnerName_old2))
      nvs_preferences->SaveOwnerName(kOwnerName_new);
  }

  //////////////////////////////////////////////////

  // audio feedback to user at start
  alarm_clock->playNote(392, 183, true);
  alarm_clock->playNote(523, 183, true);
}

// arduino loop function on core0 - High Priority one with time update tasks
void loop() {
  // note if button pressed or touchscreen touched
  bool push_button_pressed = push_button->buttonActiveDebounced();
  bool inc_button_pressed = false;
  if(inc_button != NULL)
    inc_button_pressed = inc_button->buttonActiveDebounced();
  bool dec_button_pressed = false;
  if(dec_button != NULL)
    dec_button_pressed = dec_button->buttonActiveDebounced();

  // if user presses main LED Push button, show instant response by turning On LED
  LedFeedback(push_button_pressed);

  // if a button or touchscreen is pressed then take action
  if((inactivity_millis >= kUserInputDelayMs) && (push_button_pressed || inc_button_pressed || dec_button_pressed || (ts != NULL && ts->IsTouched()))) {
    bool ts_input = (ts != NULL && ts->IsTouched());
    // show instant response by turing up brightness
    display->SetMaxBrightness();

    inactivity_millis = 0;

    // instant page change action
    if(current_page == kScreensaverPage)
    { // turn off screensaver if on
      SetPage(kMainPage);
    }
    else if(ts_input) {
      // Touch screen input
      if(current_page == kAlarmSetPage)
      { // if on alarm page, then take alarm set page user inputs
        display->SetAlarmScreen(/* process_user_input */ true, /* inc_button_pressed */ false, /* dec_button_pressed */ false, /* push_button_pressed */ false);
      }
      else // all other pages
      { // if not on alarm page and user clicked somewhere, get touch input
        Cursor user_input_cursor = display->CheckButtonTouch();
        if(user_input_cursor != kCursorNoSelection) {
          display->DisplayCursorHighlight(/*highlight_On = */ false);
          current_cursor = user_input_cursor;
          ButtonClickAction();
        }
      }
    }
    // push/big LED button click action
    else if(push_button_pressed) {
      PrintLn("push_button");
      ButtonClickAction();
    }
    else if(inc_button_pressed || dec_button_pressed) {
      if(inc_button_pressed) {
        PrintLn("inc_button_pressed");
        dec_button_pressed = false; // just having things clean, only have 1 button pressed at a time
      }
      else
        PrintLn("dec_button_pressed");
      // inc/dec button action
      if(current_page != kAlarmSetPage)
        MoveCursor(inc_button_pressed);
      else
        display->SetAlarmScreen(/* process_user_input */ true, /* inc_button_pressed */ inc_button_pressed, /* dec_button_pressed */ dec_button_pressed, /* push_button_pressed */ false);
    }

    // show firmware updated info only for the first time user uses the device
    firmware_updated_flag_user_information = false;
  }

  // new second! Update Time!
  if (rtc->rtc_hw_sec_update_) {
    rtc->rtc_hw_sec_update_ = false;

    #if defined(WIFI_IS_USED)
    // if time is lost because of power failure
    if((rtc->year() < 2024) && !(wifi_stuff->could_not_connect_to_wifi_) && !(wifi_stuff->incorrect_zip_code)) {
      // PrintLn("**** Update RTC HW Time from NTP Server ****");
      // update time from NTP server
      AddSecondCoreTaskIfNotThere(kUpdateTimeFromNtpServer);
      WaitForExecutionOfSecondCoreTask();
    }

    // auto update time at 45th second of every hour
    if(!(wifi_stuff->incorrect_zip_code) && (rtc->minute() == 0) && (rtc->second() == 45)) {
      // update time from NTP server
      AddSecondCoreTaskIfNotThere(kUpdateTimeFromNtpServer);
      // PrintLn("Get Time Update from NTP Server");
    }
    #endif

    // new minute!
    if (rtc->rtc_hw_min_update_) {
      rtc->rtc_hw_min_update_ = false;

      // PrintLn("New Minute!");

      // Activate Buzzer if Alarm Time has arrived
      if((rtc->year() >= 2024) && alarm_clock->MinutesToAlarm() == 0) {
        // update time to be shown on alarm triggered screen
        PrepareTimeDayDateArrays();
        // go to buzz alarm function and show alarm triggered screen!
        alarm_clock->BuzzAlarmFn();
        // returned from Alarm Triggered Screen and Good Morning Screen
        // set main page
        SetPage(kMainPage);
        inactivity_millis = 0;
      }

      // if screensaver is On, then update time on it
      if(current_page == kScreensaverPage) {
        display->refresh_screensaver_canvas_ = true;
        display->new_minute_ = true;
        // every new hour, show main page
        if(rtc->minute() == 0) {
          SetPage(kMainPage);
          inactivity_millis = 0;
        }
      }

      #if defined(WIFI_IS_USED)
        // try to get weather info 2 mins before alarm time
        if(alarm_clock->alarm_ON_ && (inactivity_millis > kInactivityMillisLimit) && !(wifi_stuff->incorrect_zip_code) && (alarm_clock->MinutesToAlarm() == 2)) {
          AddSecondCoreTaskIfNotThere(kGetWeatherInfo);
        }

        // check for firmware update everyday
        if(rtc->todays_minutes == ota_update_minute) {

          // reset wifi_stuff->incorrect_zip_code once everyday
          if(wifi_stuff->incorrect_zip_code)
            wifi_stuff->incorrect_zip_code = false;

          AddSecondCoreTaskIfNotThere(kFirmwareVersionCheck);
          WaitForExecutionOfSecondCoreTask();
        }

        // auto disconnect wifi if connected and inactivity millis is over limit
        if(wifi_stuff->wifi_connected_ && (inactivity_millis > kInactivityMillisLimit)) {
          AddSecondCoreTaskIfNotThere(kDisconnectWiFi);
        }

        // reset wifi_stuff->could_not_connect_to_wifi_ after 5 mins
        if(wifi_stuff->could_not_connect_to_wifi_) {
          if(wifi_stuff->mins_from_last_wifi_connect_try_ < 5)
            wifi_stuff->mins_from_last_wifi_connect_try_++;
          else
            wifi_stuff->could_not_connect_to_wifi_ = false;
        }
      #endif

      // set rgb led strip
      RunRgbLedAccordingToSettings();
    }

    // prepare date and time arrays
    PrepareTimeDayDateArrays();

    // update time on main page
    if(current_page == kMainPage)
      display->DisplayTimeUpdate();

    // serial print RTC Date Time
    // SerialPrintRtcDateTime();

    // if SAP is on, then track got_SAP_user_input_ to stop SAP
    if(((current_page == kSoftApInputsPage) || (current_page == kLocationInputsLocalServerPage)) && wifi_stuff->got_SAP_user_input_) {
      current_cursor = kPageSaveButton;
      ButtonClickAction();
      inactivity_millis = 0;
    }

    // check for inactivity
    if(inactivity_millis > (((current_page == kSoftApInputsPage) || (current_page == kLocationInputsLocalServerPage)) ? 15 * kInactivityMillisLimit : kInactivityMillisLimit)) {
      // if softap server is on, then end it
      if(current_page == kSoftApInputsPage)
        AddSecondCoreTaskIfNotThere(kStopSetWiFiSoftAP);
      else if(current_page == kLocationInputsLocalServerPage)
        AddSecondCoreTaskIfNotThere(kStopLocationInputsLocalServer);
      display->ReAdjustDisplayBrightness();
      // auto disconnect wifi if connected and inactivity millis is over limit
      if(wifi_stuff->wifi_connected_ && second_core_tasks_queue.empty()) {
        AddSecondCoreTaskIfNotThere(kDisconnectWiFi);
      }
      // turn screen saver On
      if(current_page != kScreensaverPage)
        SetPage(kScreensaverPage);
    }

    #if defined(WIFI_IS_USED)
      // update firmware if available
      if(wifi_stuff->firmware_update_available_) {
        PrintLn("**** Web OTA Firmware Update ****");
        // set Web OTA Update Pagte
        SetPage(kFirmwareUpdatePage);
        // Firmware Update
        wifi_stuff->UpdateFirmware();
        // set back main page if Web OTA Update unsuccessful
        SetPage(kMainPage);
      }
    #endif

    // reset watchdog
    ResetWatchdog();

    #ifdef MORE_LOGS
    // print fps
    if(debug_mode && current_page == kScreensaverPage) {
      PrintLn("FPS: ", frames_per_second);
      frames_per_second = 0;
    }
    #endif
  }

  // make screensaver motion
  if(current_page == kScreensaverPage) {
    display->Screensaver();
    #ifdef MORE_LOGS
    if(debug_mode) frames_per_second++;
    #endif
  }

  // accept user serial inputs
  if (Serial.available() != 0)
    SerialUserInput();

  #if defined(ESP32_SINGLE_CORE)
    // ESP32_S2_MINI is single core MCU
    loop1();
  #endif

  // if(current_page == kScreensaverPage) {
  //   /*
  //   First we configure the wake up source
  //   We set our ESP32 to wake up for an external trigger.
  //   There are two types for ESP32, ext0 and ext1 .
  //   ext0 uses RTC_IO to wakeup thus requires RTC peripherals
  //   to be on while ext1 uses RTC Controller so doesnt need
  //   peripherals to be powered on.
  //   Note that using internal pullups/pulldowns also requires
  //   RTC peripherals to be turned on.
  //   */
  //   if(digitalRead(SQW_INT_PIN))
  //     esp_sleep_enable_ext0_wakeup((gpio_num_t)SQW_INT_PIN, 0); //1 = High, 0 = Low
  //   else
  //     esp_sleep_enable_ext0_wakeup((gpio_num_t)SQW_INT_PIN, 1); //1 = High, 0 = Low
  //   //Go to sleep now
  //   Serial.println("Going to light sleep now");
  //   //esp_deep_sleep_start();
  //   Serial.flush();
  //   esp_light_sleep_start();
  // }
}

// arduino loop function on core1 - low priority one with wifi weather update task
void loop1() {
  ResetWatchdog();

  // color LED Strip sequentially
  if(rgb_led_strip_on && (current_led_strip_color != display->ColorPickerWheel(/*pick_new =*/ false)))
    SetRgbStripColor(display->ColorPickerWheel(/*pick_new =*/ false), /* set_color_sequentially = */ true);

  // code to try executing the task a few times until success
  uint8_t try_counts = 0;
  const uint8_t kTryCountsLimit = 3;

  // run the core only to do specific not time important operations
  while(!second_core_tasks_queue.empty())
  {
    ResetWatchdog();

    SecondCoreTask current_task = second_core_tasks_queue.front();
    // PrintLn("CPU", xPortGetCoreID());

    bool success = false;

    if(current_task == kGetWeatherInfo && (!wifi_stuff->got_weather_info_ || wifi_stuff->last_fetch_weather_info_time_ms_ == 0 || millis() - wifi_stuff->last_fetch_weather_info_time_ms_ > wifi_stuff->kFetchWeatherInfoMinIntervalMs)) {
      // get today's weather info
      success = wifi_stuff->GetTodaysWeatherInfo();
    }
    else if(current_task == kUpdateTimeFromNtpServer) {
      // get time from NTP server
      success = wifi_stuff->GetTimeFromNtpServer();
      if(success)
        display->redraw_display_ = true;
    }
    else if(current_task == kConnectWiFi) {
      success = wifi_stuff->TurnWiFiOn();
    }
    else if(current_task == kDisconnectWiFi) {
      wifi_stuff->TurnWiFiOff();
      success = !(wifi_stuff->wifi_connected_);
    }
    else if(current_task == kScanNetworks) {
      success = wifi_stuff->WiFiScanNetworks();
    }
    else if(current_task == kStartSetWiFiSoftAP) {
      wifi_stuff->StartSetWiFiSoftAP();
      success = true;
    }
    else if(current_task == kStopSetWiFiSoftAP) {
      wifi_stuff->StopSetWiFiSoftAP();
      success = true;
    }
    else if(current_task == kStartLocationInputsLocalServer) {
      wifi_stuff->StartSetLocationLocalServer();
      success = true;
    }
    else if(current_task == kStopLocationInputsLocalServer) {
      wifi_stuff->StopSetLocationLocalServer();
      success = true;
    }
    else if(current_task == kFirmwareVersionCheck) {
      wifi_stuff->TurnWiFiOn();
      ResetWatchdog();
      if(!(wifi_stuff->wifi_connected_))
        wifi_stuff->TurnWiFiOn();
      ResetWatchdog();
      wifi_stuff->FirmwareVersionCheck();
      success = true;
    }

    try_counts++;

    // done processing the task
    if(success || try_counts >= kTryCountsLimit) {
      second_core_tasks_queue.pop();
      delay(1);   // a delay to avoid race condition in dual core MCUs
      second_core_task_added_flag_array[current_task] = false;
      // try counts for next task
      if(!second_core_tasks_queue.empty())
        try_counts = 0;
    }
  }

  // const int kTouchActiveThreshold = 50000;
  // int touchReadPin5 = touchRead(TOUCH_PIN_5);
  // if(touchReadPin5 > kTouchActiveThreshold) {
  //   PrintLn("TOUCHED TOUCH_PIN_5");
  //   LedFeedbackOnOff();
  // }
}

#if defined(ESP32_DUAL_CORE)
void Task1code( void * parameter) {
  for(;;) 
    loop1();
}
#endif

void WaitForExecutionOfSecondCoreTask() {
  #if defined(ESP32_SINGLE_CORE)
    // ESP32_S2_MINI is single core MCU
    loop1();
  #elif defined(ESP32_DUAL_CORE)
    unsigned long time_start = millis();
    while (!second_core_tasks_queue.empty() && millis() - time_start <  kWatchdogTimeoutMs - 2000) {
      delay(10);
    }
  #endif
}

// initialize RGB LED requires NVS Preferences to be loaded
void InitializeRgbLed() {
  if(rgb_led_strip != NULL) {
    rgb_led_strip->clear();
    delete rgb_led_strip;
  }
  rgb_strip_led_count = nvs_preferences->RetrieveRgbStripLedCount();
  rgb_strip_led_brightness = nvs_preferences->RetrieveRgbStripLedBrightness();
  rgb_led_strip = new Adafruit_NeoPixel(rgb_strip_led_count, RGB_LED_STRIP_PIN(), NEO_GRB + NEO_KHZ800);
  rgb_led_strip->begin();
  autorun_rgb_led_strip_mode = (RgbLedMode)nvs_preferences->RetrieveAutorunRgbLedStripMode();
}

void RunRgbLedAccordingToSettings() {
  // set rgb led strip
  if(autorun_rgb_led_strip_mode == RGB_LED_AUTO_SUN_DOWN) { // run rgb led strip all evening + night
    if(rtc->todays_minutes >= kEveningTimeMinutes || rtc->todays_minutes < kDayTimeMinutes)
      TurnOnRgbStrip();
    else
      TurnOffRgbStrip();
  }
  else if(autorun_rgb_led_strip_mode == RGB_LED_AUTO_EVENING) {  // // run rgb led strip all evening only
    if(rtc->todays_minutes >= kEveningTimeMinutes && rtc->todays_minutes < night_time_minutes)
      TurnOnRgbStrip();
    else
      TurnOffRgbStrip();
  }
  else if(autorun_rgb_led_strip_mode == RGB_LED_MANUAL_ON)
    TurnOnRgbStrip();
  else
    TurnOffRgbStrip();
}

const char* RgbLedSettingString() {
  switch (autorun_rgb_led_strip_mode) {
    case RGB_LED_MANUAL_OFF: return kManualOffStr;
    case RGB_LED_MANUAL_ON: return kManualOnStr;
    case RGB_LED_AUTO_EVENING: return kEveningStr;
    case RGB_LED_AUTO_SUN_DOWN: return kSunDownStr;
    default: return kManualOffStr;
  }
}

// GLOBAL VARIABLES AND FUNCTIONS

uint8_t My_Hw_Version = 0x00;   // Set My_Hw_Version using nvs_preferences.h to get correct pins for your hardware!

bool use_photoresistor = false;

int current_rgb_led_strip_index = 0;

RgbLedMode autorun_rgb_led_strip_mode = RGB_LED_AUTO_EVENING;

// minute of day at which to dim display to night time brightness if not using a LDR
uint16_t night_time_minutes = 1320;

// debug mode turned On by pulling debug pin Low
bool debug_mode = false;

// firmware updated flag user information
bool firmware_updated_flag_user_information = false;

// all display buttons vector
std::vector<std::vector<DisplayButton*>> display_pages_vec(kNoPageSelected);

// CPU Speed for ESP32 CPU
uint8_t cpu_speed_mhz = 80;

// counter to note user inactivity seconds
elapsedMillis inactivity_millis = 0;

// Display Visible Data Structure variables
DisplayData new_display_data_ { "", "", "", "", true, false, true }, displayed_data_ { "", "", "", "", true, false, true };

// current page on display
ScreenPage current_page = kMainPage;

// current cursor highlight location on page
Cursor current_cursor = kCursorNoSelection;

// second core current task queue
std::queue<SecondCoreTask> second_core_tasks_queue;
// second core task added flag
bool second_core_task_added_flag_array[kNoTask];

// function to safely add second core task if not already there
void AddSecondCoreTaskIfNotThere(SecondCoreTask task) {
  if(!second_core_task_added_flag_array[task]) {
    second_core_tasks_queue.push(task);
    second_core_task_added_flag_array[task] = true;
  }
}

int AvailableRam() {
  // https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/system/misc_system_api.html
  return esp_get_free_heap_size();
}

int MinFreeRam() {
  // https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/system/misc_system_api.html
  return esp_get_minimum_free_heap_size();
}

void SerialInputWait() {
  while (Serial.available() == 0) // delay until something is received via serial
    delay(20);
  delay(20);  // give sometime to MCU to read serial input
}

void SerialInputFlush() {
  delay(20);  // give sometime to MCU to read serial input
  // if MCU received something, get all of it and discard it
  while (Serial.available()) {
    Serial.read();
    delay(20);  // give sometime to MCU to read next serial input
  }
  // nothing arrived for 20 ms, break off
}

void SerialTimeStampPrefix() {
  Serial.print(millis());
  Serial.print(kCharSpace);
  Serial.print('(');
  if(rtc != NULL) {
    Serial.print(rtc->hour());
    Serial.print(kCharColon);
    if(rtc->minute() < 10) Serial.print(kCharZero);
    Serial.print(rtc->minute());
    Serial.print(kCharColon);
    if(rtc->second() < 10) Serial.print(kCharZero);
    Serial.print(rtc->second());
    Serial.print(kCharSpace);
    if(rtc->hourModeAndAmPm() == 1)
      Serial.print(kAmLabel);
    else if(rtc->hourModeAndAmPm() == 2)
      Serial.print(kPmLabel);
  }
  Serial.print(" :i");
  // if(inactivity_millis < 100) Serial.print(kCharZero);
  // if(inactivity_millis < 10) Serial.print(kCharZero);
  Serial.print(inactivity_millis);
  // int freeRam = AvailableRam();
  // Serial.print(": FreeRAM "); Serial.print(freeRamf); Serial.print('B');
  Serial.print(": MinRAM "); Serial.print(MinFreeRam());
  Serial.print("B)");
  Serial.print(kCharSpace);
  Serial.flush();
}

void PrintLn(const char* someText1, const char* someText2) {
  SerialTimeStampPrefix();
  if(someText1 != nullptr)
    Serial.print(someText1);
  Serial.print(kCharSpace);
  if(someText2 != nullptr)
    Serial.print(someText2);
  Serial.println();
  Serial.flush();
}
void PrintLn(const char* someText1) {
  SerialTimeStampPrefix();
  if(someText1 != nullptr)
    Serial.print(someText1);
  Serial.println();
  Serial.flush();
}
void PrintLn(const char* someText1, int someInt) {
  SerialTimeStampPrefix();
  Serial.print(someText1);
  Serial.print(kCharSpace);
  Serial.println(someInt);
  Serial.flush();
}
void PrintLn(std::string someTextStr1, std::string someTextStr2) {
  SerialTimeStampPrefix();
  Serial.print(someTextStr1.c_str());
  Serial.print(kCharSpace);
  Serial.println(someTextStr2.c_str());
  Serial.flush();
}
void PrintLn(std::string &someTextStr1, std::string &someTextStr2) {
  SerialTimeStampPrefix();
  Serial.print(someTextStr1.c_str());
  Serial.print(kCharSpace);
  Serial.println(someTextStr2.c_str());
  Serial.flush();
}
void PrintLn(std::string &someTextStr1, int someInt) {
  SerialTimeStampPrefix();
  Serial.println(someTextStr1.c_str());
  Serial.print(kCharSpace);
  Serial.println(someInt);
  Serial.flush();
}
void PrintLn(std::string &someTextStr1) {
  SerialTimeStampPrefix();
  Serial.println(someTextStr1.c_str());
  Serial.flush();
}
void PrintLn(int someInt) {
  SerialTimeStampPrefix();
  Serial.println(someInt);
  Serial.flush();
}
void PrintLn() {
  SerialTimeStampPrefix();
  Serial.println();
  Serial.flush();
}

void PrepareTimeDayDateArrays() {
  // HH:MM
  snprintf(new_display_data_.time_HHMM, kHHMM_ArraySize, "%d:%02d", rtc->hour(), rtc->minute());
  // :SS
  snprintf(new_display_data_.time_SS, kSS_ArraySize, ":%02d", rtc->second());
  if(rtc->hourModeAndAmPm() == 0)
    new_display_data_._12_hour_mode = false;
  else if(rtc->hourModeAndAmPm() == 1) {
    new_display_data_._12_hour_mode = true;
    new_display_data_.pm_not_am = false;
  }
  else {
    new_display_data_._12_hour_mode = true;
    new_display_data_.pm_not_am = true;
  }
  // Mon dd Day
  snprintf(new_display_data_.date_str, kDateArraySize, "%s  %d  %s", kDaysTable_[rtc->dayOfWeek() - 1], rtc->day(), kMonthsTable[rtc->month() - 1]);
  if(alarm_clock->alarm_ON_)
    snprintf(new_display_data_.alarm_str, kAlarmArraySize, "%d:%02d %s", alarm_clock->alarm_hr_, alarm_clock->alarm_min_, (alarm_clock->alarm_is_AM_ ? kAmLabel : kPmLabel));
  else
    snprintf(new_display_data_.alarm_str, kAlarmArraySize, "%s %s", kAlarmLabel, kOffLabel);
  new_display_data_.alarm_ON = alarm_clock->alarm_ON_;
}

void SerialPrintRtcDateTime() {
  SerialTimeStampPrefix();
  // full serial print time date day array
  Serial.print(new_display_data_.time_HHMM);
  // snprintf(timeArraySec, SS_ARR_SIZE, ":%02d", second);
  Serial.print(new_display_data_.time_SS);
  if (new_display_data_._12_hour_mode) {
    Serial.print(kCharSpace);
    if (new_display_data_.pm_not_am)
      Serial.print(kPmLabel);
    else
      Serial.print(kAmLabel);
  }
  Serial.print(kCharSpace);
  Serial.print(new_display_data_.date_str);
  Serial.print(kCharSpace);
  Serial.print(rtc->year());
  Serial.print(kCharSpace);
  Serial.println(new_display_data_.alarm_str);
  Serial.flush();
}

// set watchdog time
void SetWatchdogTime(unsigned long ms) {
  PrintLn(__func__, ms);
  #if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
  // Code for version 3.x
    // // esp32 watchdog example https://iotassistant.io/esp32/fixing-error-hardware-wdt-arduino-esp32/
    // esp_task_wdt_deinit(); //wdt is enabled by default, so we need to deinit it first
    // esp_task_wdt_config_t twdt_config = {
    //     .timeout_ms = ms,
    //     .idle_core_mask = (1 << CONFIG_FREERTOS_NUMBER_OF_CORES) - 1,    // Bitmask of all cores
    //     .trigger_panic = true,
    //     .panic_action = ESP_TASK_WDT_PANIC_ACTION_RESTART,
    // };
    // esp_task_wdt_init(&twdt_config); //enable panic so ESP32 restarts
    // esp_task_wdt_add(NULL); //add current thread to WDT watch
    watchdog_timer = timerBegin(1000);                     //timer 1khz resolution
    timerAttachInterrupt(watchdog_timer, &resetModule);       //attach callback
    timerAlarm(watchdog_timer, ms, false, 0);  //set time in us
  #else
  // Code for version 2.x
    // https://iotassistant.io/esp32/enable-hardware-watchdog-timer-esp32-arduino-ide/
    // https://docs.espressif.com/projects/esp-idf/en/stable/esp32s2/api-reference/system/wdts.html
    // https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/system/wdts.html
    esp_task_wdt_init(ms / 1000, true); //enable panic so ESP32 restarts
    esp_task_wdt_add(NULL); //add current thread to WDT watch
  #endif
}

// reset watchdog within time so it does not reboot system
void ResetWatchdog() {
  // reset MCU if not in debug mode
  if(!debug_mode) {
    #if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
    // Code for version 3.x
      timerWrite(watchdog_timer, 0);  //reset timer (feed watchdog)
    #else
    // Code for version 2.x
      // https://iotassistant.io/esp32/enable-hardware-watchdog-timer-esp32-arduino-ide/
      // https://docs.espressif.com/projects/esp-idf/en/stable/esp32s2/api-reference/system/wdts.html
      esp_task_wdt_reset();
    #endif
  }
}

void sortedPush(int16_t* samples_arr, int16_t val, int len) {
  int i = 0;
  while(i < len) {
    // is current index value greater than val? shift all arr values right, push val at current index
    if(samples_arr[i] > val) {
      // shift all arr vals right
      for (int j = len; j > i; j--) {
        samples_arr[j] = samples_arr[j - 1];
      }
      // push val
      samples_arr[i] = val;
      return;
    }
    else {
      i++;
    }
  }
  // val is larger than all array values
  samples_arr[len] = val;
}

void CalibrateTouchscreenFn() {
  display->SetMaxBrightness();
  if(ts != NULL) {
    const int kNumOfSamples = 10, kNumOfEdges = 4, kDivider = 5;
    int16_t samples[kNumOfSamples];
    float med_avg_val = 0;
    int16_t x_left_avg = 0, x_right_avg = 0, y_top_avg = 0, y_btm_avg = 0;
    /* edges -  0 = top
                1 = btm
                2 = left
                3 = right
    */
    int sample_num = 0, edge_num = 0;
    std::string log_str;
    // top edge
    int16_t x0 = kTftWidth / kDivider, y0 = kTftHeight / kDivider, x1 = (kDivider-1) * kTftWidth / kDivider, y1 = kTftHeight / kDivider;
    display->TouchCalibrationScreen(x0, y0, x1, y1, /*touched*/ false, /*redraw*/ true); // first draw
    PrintLn();
    while(1) {
      ResetWatchdog();
      int16_t x = -1, y = -1;
      if(!(ts->GetUncalibratedTouch(x, y))) {
        display->TouchCalibrationScreen(x0, y0, x1, y1, /*touched*/ false, /*redraw*/ false);   // show no touch
      }
      else {
        // second reading is better
        ts->GetUncalibratedTouch(x, y);
        if(edge_num <= 1)
          sortedPush(samples, y, sample_num);
        else
          sortedPush(samples, x, sample_num);
        {
          for (int g = 0; g < sample_num+1; g++) {
            Serial.print(samples[g]); Serial.print(" ");
          }
          Serial.println();
        }
        log_str = "edge_num " + std::to_string(edge_num) + ", sample_num " + std::to_string(sample_num) + ", x " + std::to_string(x) + ", y " + std::to_string(y);
        PrintLn(log_str);
        display->TouchCalibrationScreen(x0, y0, x1, y1, /*touched*/ true, /*redraw*/ false);   // show touch
        delay(kUserInputDelayMs);
        display->TouchCalibrationScreen(x0, y0, x1, y1, /*touched*/ false, /*redraw*/ false);   // show no touch
        delay(kUserInputDelayMs);
        sample_num++;
        if(sample_num >= kNumOfSamples) {
          // calculate median average (average of middle 6 values)
          med_avg_val = 0;
          sample_num = 2;
          while(sample_num < 8) {
            med_avg_val += ((float)samples[sample_num]) / 6;
            sample_num++;
          }
          // process samples
          switch(edge_num) {
            case 0:   // top
              y_top_avg = med_avg_val;
            case 1:   // bottom
              y_btm_avg = med_avg_val;
              break;
            case 2:   // left
              x_left_avg = med_avg_val;
              break;
            case 3:   // right
              x_right_avg = med_avg_val;
              break;
          }
          sample_num = 0;
          med_avg_val = 0;
          edge_num++;
          PrintLn();
          LedFeedback(true);
          if(edge_num < kNumOfEdges) {
            // next edge
            if(edge_num == 1) {  // bottom
              x0 = kTftWidth / kDivider, y0 = (kDivider-1) * kTftHeight / kDivider, x1 = (kDivider-1) * kTftWidth / kDivider, y1 = (kDivider-1) * kTftHeight / kDivider;
            }
            else if(edge_num == 2) {   // left
              x0 = kTftWidth / kDivider, y0 = kTftHeight / kDivider, x1 = kTftWidth / kDivider, y1 = (kDivider-1) * kTftHeight / kDivider;
            }
            else {  // right
              x0 = (kDivider-1) * kTftWidth / kDivider, y0 = kTftHeight / kDivider, x1 = (kDivider-1) * kTftWidth / kDivider, y1 = (kDivider-1) * kTftHeight / kDivider;
            }
            // screen with no line
            display->TouchCalibrationScreen(kTftWidth * 2, 0, kTftWidth * 2, kTftHeight - 1, /*touched*/ false, /*redraw*/ true); // redraw
            delay(1000);
            LedFeedback(false);
            display->TouchCalibrationScreen(x0, y0, x1, y1, /*touched*/ false, /*redraw*/ true);  // redraw
          }
          else {
            // screen with no line
            display->TouchCalibrationScreen(kTftWidth * 2, 0, kTftWidth * 2, kTftHeight - 1, /*touched*/ false, /*redraw*/ true, /*calibration_done*/ true); // redraw
            delay(2000);
            LedFeedback(false);
            break;
          }
        }
      }
    }
    // touch calibration inputs received

    // flip touchscreen if y_top_avg > y_btm_avg
    bool touchscreen_flip;
    if(y_top_avg > y_btm_avg)
      touchscreen_flip = true;
    else
      touchscreen_flip = false;
    int16_t temp_val = y_top_avg;
    y_top_avg = y_btm_avg;
    y_btm_avg = temp_val;
    log_str = "x_left_avg=" + std::to_string(x_left_avg) + ", x_right_avg=" + std::to_string(x_right_avg) + ", y_top_avg=" + std::to_string(y_top_avg) + ", y_btm_avg=" + std::to_string(y_btm_avg) + ", touchscreen_flip=" + std::to_string(touchscreen_flip);
    PrintLn(log_str);

    // for ESP32 ADC touchscreen
    // x_left_avg=209, x_right_avg=715, y_top_avg=274, y_btm_avg=757, touchscreen_flip=1
    // xMin, xMax, yMin, yMax = 41, 881, 113, 914

    // for XPT2046
    // x_left_avg=925, x_right_avg=3004, y_top_avg=933, y_btm_avg=3070, touchscreen_flip=1
    // xMin, xMax, yMin, yMax = 232, 3686, 221, 3767

    // calculate screen min max calibration values
    int16_t xMin = map(0, kTftWidth / kDivider, (kDivider-1) * kTftWidth / kDivider, x_left_avg, x_right_avg);
    int16_t xMax = map(kTftWidth - 1, kTftWidth / kDivider, (kDivider-1) * kTftWidth / kDivider, x_left_avg, x_right_avg);
    int16_t yMin = map(0, kTftHeight / kDivider, (kDivider-1) * kTftHeight / kDivider, y_top_avg, y_btm_avg);
    int16_t yMax = map(kTftHeight - 1, kTftHeight / kDivider, (kDivider-1) * kTftHeight / kDivider, y_top_avg, y_btm_avg);
    log_str = "xMin, xMax, yMin, yMax = " + std::to_string(xMin) + ", " + std::to_string(xMax) + ", " + std::to_string(yMin) + ", " + std::to_string(yMax);
    PrintLn(log_str);

    // set new touchscreen calibration values
    ts->SetTouchscreenCalibration(xMin, xMax, yMin, yMax);
    ts->touchscreen_flip = touchscreen_flip;

    // test touchscreen calibration
    bool accurate_calibration = TestTouchscreenCalibrationFn();

    if(accurate_calibration) {
      LedFeedback(true);
      /* save calibration to NVS memory*/
      nvs_preferences->SaveTouchScreenCalibration(xMin, xMax, yMin, yMax);
      nvs_preferences->SaveTouchscreenFlip(ts->touchscreen_flip);

      // display message
      display->DisplayMessage(std::string("New Calibration Saved"), std::string(""));
      delay(2000);
      LedFeedback(false);
    }
    else {
      // retrieve old calibration values
      nvs_preferences->RetrieveTouchScreenCalibration(xMin, xMax, yMin, yMax);
      touchscreen_flip = nvs_preferences->RetrieveTouchscreenFlip();
      // set old touchscreen calibration values
      ts->SetTouchscreenCalibration(xMin, xMax, yMin, yMax);
      ts->touchscreen_flip = touchscreen_flip;

      // display message
      display->DisplayMessage(std::string("Re-Calibration Rejected"), std::string(""));
      delay(2000);
    }
  }
}

bool TestTouchscreenCalibrationFn() {
  display->SetMaxBrightness();
  if(ts != NULL) {
    // test calibration
    const int kNumOfSamples = 10, kDivider = 5;
    uint8_t test_num = 0, accurate_touches = 0;

    // first target coordinate
    int16_t x_target = (test_num / 2 + 1) * kTftWidth / (kNumOfSamples / 2 + 1);
    int16_t y_target = kTftHeight / kDivider;
    display->TouchCalibrationScreenTest(x_target, y_target, -1, -1, true); // first draw

    // begin touchscreen calibration test
    while(1) {
      ResetWatchdog();
      if(ts->GetTouchedPixel()->is_touched) {
        // show touch
        if(display->TouchCalibrationScreenTest(x_target, y_target, ts->GetTouchedPixel()->x, ts->GetTouchedPixel()->y, /* redraw = */ false))
          accurate_touches++;
        // wait
        delay(2 * kUserInputDelayMs);
        test_num++;
        if(test_num < kNumOfSamples) {
          // new target coordinate
          x_target = (test_num / 2 + 1) * kTftWidth / (kNumOfSamples / 2 + 1);
          if((test_num & 0x01) == 0x00)
            y_target = kTftHeight / kDivider;
          else
            y_target = (kDivider-1) * kTftHeight / kDivider;
          display->TouchCalibrationScreenTest(x_target, y_target, -1, -1, false); // new point
          // wait
          delay(2 * kUserInputDelayMs);
        }
        else {
          break;
        }
      }
    }

    delay(2000);
    
    // note touch accuracy
    bool accurate_calibration = ((int)accurate_touches == kNumOfSamples ? true : false);
    if(accurate_calibration)
      display->DisplayMessage(std::string("Good Calibration"), std::string(""));
    else
      display->DisplayMessage(std::string("Bad Calibration"), std::string(""));
    delay(2000);

    return accurate_calibration;
  }
  else {
    display->TouchCalibrationScreenTest(0, 0, -1, -1, true); // first draw
    delay(1000);
  }
  return false;
}

/* Take user inputs and configure

  Mostly made for debug purpose

  To enable more logs, enable #define MORE_LOGS in configuration.h file
*/
void SerialUserInput() {
  // take user input
  char input = Serial.read();
  SerialInputFlush();
  // acceptable user input
  std::string s = "User input: ";
  s += input;
  PrintLn(s);

  // process user input
  switch (input) {
    case 'a':   // show alarm triggered screen
      #ifdef MORE_LOGS
      PrintLn("**** Show Alarm Triggered Screen ****");
      #endif
      display->SetMaxBrightness();
      // start alarm triggered page
      SetPage(kAlarmTriggeredPage);
      delay(1000);
      display->AlarmTriggeredScreen(false, 24);
      delay(2000);
      display->AlarmTriggeredScreen(false, 12);
      delay(2000);
      display->AlarmTriggeredScreen(false, 5);
      delay(2000);
      // set main page back
      SetPage(kMainPage);
      inactivity_millis = 0;
      break;
    case 'b':   // RGB LED brightness
      {
        #ifdef MORE_LOGS
        PrintLn("**** Set RGB Brightness [0-255] ****");
        #endif
        SerialInputWait();
        int brightnessVal = Serial.parseInt();
        SerialInputFlush();
        nvs_preferences->SaveRgbStripLedBrightness(brightnessVal);
        InitializeRgbLed();
        RunRgbLedAccordingToSettings();
      }
      break;
    case 'c':   // connect/disconnect WiFi
      if(wifi_stuff->wifi_connected_) {
        AddSecondCoreTaskIfNotThere(kDisconnectWiFi);
      }
      else {
        AddSecondCoreTaskIfNotThere(kConnectWiFi);
        inactivity_millis = 0;
      }
      break;
    case 'd':   // flip touchscreen
      nvs_preferences->SaveTouchscreenFlip(!nvs_preferences->RetrieveTouchscreenFlip());
      ts->touchscreen_flip = nvs_preferences->RetrieveTouchscreenFlip();
      break;
    case 'e':   // toggle rtc type DS1307/DS3231
      {
        uint8_t rtc_type = nvs_preferences->RetrieveRtcType();
        if(rtc_type == URTCLIB_MODEL_DS1307)
          nvs_preferences->SaveRtcType(URTCLIB_MODEL_DS3231);
        else  // URTCLIB_MODEL_DS3231
          nvs_preferences->SaveRtcType(URTCLIB_MODEL_DS1307);
        rtc->RtcSetup();
      }
      break;
    case 'f':   // toggle 12 / 24 hour mode
      {
        #ifdef MORE_LOGS
        PrintLn("**** toggle 12 / 24 hour mode ****");
        uint8_t hr_mode = rtc->hourModeAndAmPm();
        std::string hr_mode_str = "current mode = ";
        if(hr_mode == 0)
          hr_mode_str += "24-hr mode";
        else if(hr_mode == 1)
          hr_mode_str += "12-hr mode AM";
        else
          hr_mode_str += "12-hr mode PM";
        PrintLn(hr_mode_str);
        #endif
        if(rtc->hourModeAndAmPm() == 0)
          rtc->set_12hour_mode(true);
        else
          rtc->set_12hour_mode(false);
        PrintLn(rtc->hourModeAndAmPm());
        #ifdef MORE_LOGS
        hr_mode = rtc->hourModeAndAmPm();
        hr_mode_str = "new mode = ";
        if(hr_mode == 0)
          hr_mode_str += "24-hr mode";
        else if(hr_mode == 1)
          hr_mode_str += "12-hr mode AM";
        else
          hr_mode_str += "12-hr mode PM";
        PrintLn(hr_mode_str);
        #endif
      }
      break;
    case 'g':   // good morning
      display->SetMaxBrightness();
      display->GoodMorningScreen();
      SetPage(kMainPage);
      break;
    case 'h':   // toggle TOUCHSCREEN type
      {
        uint8_t touchscreen_type = nvs_preferences->RetrieveTouchscreenType();
        touchscreen_type++;
        if(touchscreen_type > 2)
          touchscreen_type = 0;
        nvs_preferences->SaveTouchscreenType(touchscreen_type);
        delete ts;
        ts = NULL;
        if(touchscreen_type) {
          ts = new Touchscreen();
        }
      }
      break;
    case 'i':   // set WiFi details
      {
        // increase watchdog timeout to 90s
        SetWatchdogTime(kWatchdogTimeoutOtaUpdateMs);

        #ifdef MORE_LOGS
        PrintLn("**** Enter WiFi Details ****");
        #endif
        String inputStr;
        #ifdef MORE_LOGS
        PrintLn("SSID:");
        #endif
        SerialInputWait();
        inputStr = Serial.readString();
        wifi_stuff->wifi_ssid_ = "";
        for (int i = 0; i < min(inputStr.length(), kWifiSsidPasswordLengthMax); i++)
          if(inputStr[i] != '\0' && inputStr[i] != '\n')
            wifi_stuff->wifi_ssid_ = wifi_stuff->wifi_ssid_ + inputStr[i];
        Serial.println(wifi_stuff->wifi_ssid_.c_str());
        #ifdef MORE_LOGS
        PrintLn("Pwd:");
        #endif
        SerialInputWait();
        inputStr = Serial.readString();
        wifi_stuff->wifi_password_ = "";
        for (int i = 0; i < min(inputStr.length(), kWifiSsidPasswordLengthMax); i++)
          if(inputStr[i] != '\0' && inputStr[i] != '\n')
            wifi_stuff->wifi_password_ = wifi_stuff->wifi_password_ + inputStr[i];
        Serial.println(wifi_stuff->wifi_password_.c_str());
        wifi_stuff->SaveWiFiDetails();

        // set back watchdog timeout
        SetWatchdogTime(kWatchdogTimeoutMs);
      }
      break;
    case 'j':   // cycle through CPU speeds
      #ifdef MORE_LOGS
      PrintLn("**** cycle through CPU speeds ****");
      #endif
      CycleCpuFrequency();
      break;
    case 'k':   // set firmware updated flag true
      #ifdef MORE_LOGS
      PrintLn("**** set firmware updated flag true ****");
      #endif
      firmware_updated_flag_user_information = true;
      break;
    case 'l':   // CalibrateTouchscreenFn()
      CalibrateTouchscreenFn();
      // set back current page
      SetPage(current_page);
      inactivity_millis = 0;
      break;
    case 'm':   // RotateScreen();
      display->RotateScreen();
      if(ts != NULL)
          ts->SetTouchscreenOrientation();
      display->redraw_display_ = true;
      break;
    case 'n':   // get time from NTP server and set on RTC HW
      #ifdef MORE_LOGS
      PrintLn("**** Update RTC HW Time from NTP Server ****");
      #endif
      // update time from NTP server
      AddSecondCoreTaskIfNotThere(kUpdateTimeFromNtpServer);
      break;
    case 'o':   // On Screen User Text Input
      {
        #ifdef MORE_LOGS
        PrintLn("**** On Screen User Text Input ****");
        #endif
        // SetPage(kSettingsPage);
        // user input string
        std::string label = "On Screen User Text Input:";
        char returnText[kWifiSsidPasswordLengthMax + 1] = "";
        // get user input from screen
        display->GetUserOnScreenTextInput(label, returnText, /* bool numbers_only = */ false, /* bool alphabets_only = */ false);
        PrintLn(returnText);
        SetPage(kMainPage);
      }
      break;
    case 'p':   // turn ON RGB LED Strip
      TurnOnRgbStrip();
      break;
    case 'q':   // turn OFF RGB LED Strip
      TurnOffRgbStrip();
      break;
    case 'r':   // TestTouchscreenCalibrationFn()
      TestTouchscreenCalibrationFn();
      // set back current page
      SetPage(current_page);
      inactivity_millis = 0;
      break;
    case 's':   // toggle screensaver
      #ifdef MORE_LOGS
      PrintLn("**** toggle Screensaver ****");
      #endif
      if(current_page != kScreensaverPage)
        SetPage(kScreensaverPage);
      else
        SetPage(kMainPage);
      break;
    case 't':   // go to buzzAlarm Function
      #ifdef MORE_LOGS
      PrintLn("**** buzzAlarm Function ****");
      #endif
      display->SetMaxBrightness();
      // go to buzz alarm function
      alarm_clock->BuzzAlarmFn();
      // set main page back
      SetPage(kMainPage);
      inactivity_millis = 0;
      break;
    case 'u':   // Web OTA Update Available Check
      #ifdef MORE_LOGS
      PrintLn("**** Web OTA Update Available Check ****");
      #endif
      wifi_stuff->FirmwareVersionCheck();
      wifi_stuff->firmware_update_available_ = false;
      break;
    case 'v':   // Web OTA Update
      #ifdef MORE_LOGS
      PrintLn("**** Web OTA Update Check ****");
      #endif
      ResetWatchdog();
      // set Web OTA Update Pagte
      SetPage(kFirmwareUpdatePage);
      // Firmware Update
      wifi_stuff->UpdateFirmware();
      // set back main page if Web OTA Update unsuccessful
      SetPage(kMainPage);
      break;
    case 'w':   // get today's weather info
      #ifdef MORE_LOGS
      PrintLn("**** Get Weather Info ****");
      #endif
      // get today's weather info
      AddSecondCoreTaskIfNotThere(kGetWeatherInfo);
      break;
    case 'x':   // toggle RGB LED Strip Mode
      if((uint8_t)autorun_rgb_led_strip_mode < 3)
        autorun_rgb_led_strip_mode = (RgbLedMode)((uint8_t)autorun_rgb_led_strip_mode + 1);
      else
        autorun_rgb_led_strip_mode = RGB_LED_MANUAL_OFF;
      nvs_preferences->SaveAutorunRgbLedStripMode((uint8_t)autorun_rgb_led_strip_mode);
      PrintLn(RgbLedSettingString());
      break;
    case 'y':   // Remove KEY NVS Preferences
      {
        // increase watchdog timeout to 90s
        SetWatchdogTime(kWatchdogTimeoutOtaUpdateMs);

        #ifdef MORE_LOGS
        PrintLn("**** Remove NVS Preferences Key ****");
        #endif
        String inputStr;
        PrintLn("NVS Key to remove:");
        SerialInputWait();
        inputStr = Serial.readString();
        std::string remove_key = "";
        for (int i = 0; i < min(inputStr.length(), (unsigned int)15); i++)
          if(inputStr[i] != '\0' && inputStr[i] != '\n')
            remove_key = remove_key + inputStr[i];
        Serial.println(remove_key.c_str());
        nvs_preferences->RemoveKey(remove_key);

        // set back watchdog timeout
        SetWatchdogTime(kWatchdogTimeoutMs);
      }
      break;
    case 'z':   // screensaver toggle show_colored_edge_screensaver_
      {
        #ifdef MORE_LOGS
        PrintLn("**** Screensaver toggle show_colored_edge_screensaver_ ****");
        #endif
        display->show_colored_edge_screensaver_ = !display->show_colored_edge_screensaver_;
        display->refresh_screensaver_canvas_ = true;
      }
      break;
    default:
      PrintLn("Unrecognized user input");
    // case 'r':   // Set RGB LED Count
    //   {
    //     #ifdef MORE_LOGS
    //     PrintLn("**** Set RGB LED Count [0-255] ****");
    //     #endif
    //     SerialInputWait();
    //     int rgb_strip_led_count_user = Serial.parseInt();
    //     SerialInputFlush();
    //     nvs_preferences->SaveRgbStripLedCount(rgb_strip_led_count_user);
    //     InitializeRgbLed();
    //     RunRgbLedAccordingToSettings();
    //   }
    //   break;
  }
}

void CycleCpuFrequency() {
  cpu_speed_mhz = getCpuFrequencyMhz();
  // cycle through 80, 160 and 240
  if(cpu_speed_mhz == 160) setCpuFrequencyMhz(240);
  else if(cpu_speed_mhz == 240) setCpuFrequencyMhz(80);
  else setCpuFrequencyMhz(160);
  cpu_speed_mhz = getCpuFrequencyMhz();
  nvs_preferences->SaveCpuSpeed();
}

void SetRgbStripColor(uint16_t rgb565_color, bool set_color_sequentially) {
  if(!rgb_led_strip_on)
    return;

  // RGB565 to RGB888
  byte r = byte(((rgb565_color & 0xF800) >> 11) << 3);
  byte g = byte(((rgb565_color & 0x7E0) >> 5) << 2);
  byte b = byte(((rgb565_color & 0x1F)) << 3);
  uint32_t rgb888 = (uint32_t(r) << 16) + (uint32_t(g) << 8) + uint32_t(b);
  // Serial.printf("rgb565_color = 0x%X   r=%d  g=%d  b=%d    rgb888 = 0x%X\n", rgb565_color, r, g, b, rgb888);
  // color rgb_led_strip
  if(set_color_sequentially) {
    rgb_led_strip->setPixelColor(current_rgb_led_strip_index, rgb888);
    current_rgb_led_strip_index++;
    if(current_rgb_led_strip_index == rgb_strip_led_count) {
      current_rgb_led_strip_index = 0;
      // stop further color changes until screensaver color change
      current_led_strip_color = display->ColorPickerWheel(/*pick_new =*/ false);
    }
  }
  else {
    rgb_led_strip->fill(rgb888, 0, 0);
  }
  rgb_led_strip->show();
}

void TurnOnRgbStrip() {
  rgb_led_strip->setBrightness(rgb_strip_led_brightness);
  rgb_led_strip->fill(kDefaultLedStripColor, 0, 0);
  rgb_led_strip->show();
  rgb_led_strip_on = true;
  PrintLn("TurnOnRgbStrip()");
}

void TurnOffRgbStrip() {
  // rgb_led_strip->fill(0x000000, 0, 0);
  rgb_led_strip->setBrightness(0);
  rgb_led_strip->show();
  rgb_led_strip_on = false;
  // PrintLn("TurnOffRgbStrip()");
}

bool AnyButtonPressed() {
  if(push_button->buttonActiveDebounced())
    return true;
  else if((inc_button != NULL) &&  inc_button->buttonActiveDebounced())
    return true;
  else if((dec_button != NULL) &&  dec_button->buttonActiveDebounced())
    return true;
  else
    return false;
}

void SetPage(ScreenPage set_this_page) {
  SetPage(set_this_page, /* bool move_cursor_to_first_button = */ true, /* bool increment_page = */ false);
}

void SetPage(ScreenPage set_this_page, bool move_cursor_to_first_button) {
  SetPage(set_this_page, move_cursor_to_first_button, /* bool increment_page = */ false);
}

void SetPage(ScreenPage set_this_page, bool move_cursor_to_first_button, bool increment_page) {
  switch(set_this_page) {
    case kMainPage:
      // if screensaver is active then clear screensaver canvas to free memory
      if(current_page == kScreensaverPage)
        display->ScreensaverControl(false);
      current_page = set_this_page;         // new page needs to be set before any action
      if(move_cursor_to_first_button) current_cursor = kCursorNoSelection;
      display->redraw_display_ = true;
      display->DisplayTimeUpdate();
      // useful flag to show on UI the latest firmware in Settings Page
      wifi_stuff->firmware_update_available_str_ = "";
      break;
    case kScreensaverPage:
      current_page = set_this_page;      // new page needs to be set before any action
      display->ScreensaverControl(true);
      if(move_cursor_to_first_button) current_cursor = kCursorNoSelection;
      break;
    case kFirmwareUpdatePage:
      current_page = set_this_page;
      display->FirmwareUpdatePage();
      break;
    case kAlarmSetPage:
      current_page = set_this_page;     // new page needs to be set before any action
      if(move_cursor_to_first_button) current_cursor = kAlarmSetPageHour;
      // set variables for alarm set screen
      alarm_clock->var_1_ = alarm_clock->alarm_hr_;
      alarm_clock->var_2_ = alarm_clock->alarm_min_;
      alarm_clock->var_3_is_AM_ = alarm_clock->alarm_is_AM_;
      alarm_clock->var_4_ON_ = alarm_clock->alarm_ON_;
      display->SetAlarmScreen(/* process_user_input */ false, /* inc_button_pressed */ false, /* dec_button_pressed */ false, /* push_button_pressed */ false);
      break;
    case kAlarmTriggeredPage:
      current_page = set_this_page;     // new page needs to be set before any action
      display->AlarmTriggeredScreen(true, alarm_clock->alarm_long_press_seconds_);
      break;
    case kWiFiSettingsPage:
      // try to connect to WiFi
      AddSecondCoreTaskIfNotThere(kConnectWiFi);
      WaitForExecutionOfSecondCoreTask();
      // show page
      current_page = set_this_page;     // new page needs to be set before any action
      if(move_cursor_to_first_button) current_cursor = kWiFiSettingsPageScanNetworks;
      display->DisplayCurrentPage();
      break;
    case kWiFiScanNetworksPage:
      current_page = set_this_page;     // new page needs to be set before any action
      if(move_cursor_to_first_button) current_cursor = kWiFiScanNetworksPageRescan;
      display->WiFiScanNetworksPage(increment_page);
      break;
    case kSoftApInputsPage:
      current_page = set_this_page;     // new page needs to be set before any action
      if(move_cursor_to_first_button) current_cursor = kPageSaveButton;
      display->SoftApInputsPage();
      break;
    case kLocationInputsLocalServerPage:
      current_page = set_this_page;     // new page needs to be set before any action
      if(move_cursor_to_first_button) current_cursor = kPageSaveButton;
      display->LocationInputsLocalServerPage();
      break;
    default:
      current_page = set_this_page;     // new page needs to be set before any action
      if(move_cursor_to_first_button) current_cursor = display_pages_vec[current_page][0]->btn_cursor_id;
      display->DisplayCurrentPage();
  }
  delay(kUserInputDelayMs);
  display->DisplayCursorHighlight(/*highlight_On = */ true);
  LedFeedback(false);
}

void MoveCursor(bool increment) {
  // Alarm Set Page (kAlarmSetPage) increment/decrement operations do not come here
  // they are handled as a special case

  display->DisplayCursorHighlight(/*highlight_On = */ false);

  // find first and last Click button in the page
  int first_click_button_index = 0, last_click_button_index = display_pages_vec[current_page].size() - 1;
  while(display_pages_vec[current_page][first_click_button_index]->btn_type == kLabelOnlyNoClickButton)
    first_click_button_index++;
  while(display_pages_vec[current_page][last_click_button_index]->btn_type == kLabelOnlyNoClickButton)
    last_click_button_index--;
  // PrintLn("first_click_button_index = ", first_click_button_index);
  // PrintLn("last_click_button_index = ", last_click_button_index);
  // PrintLn("increment = ", increment);
  // special case of WiFi Scan Networks Page:
  bool find_next_button = false;
  if((current_page == kWiFiScanNetworksPage) && (current_cursor == kWiFiScanNetworksPageList)) {
    // move cursor inside the WiFi Networks Selection List
    if(increment) {
      if(display->current_wifi_networks_scan_page_cursor < display->kWifiScanNetworksPageItems - 1) {
        display->current_wifi_networks_scan_page_cursor++;
        PrintLn("display->current_wifi_networks_scan_page_cursor = ", display->current_wifi_networks_scan_page_cursor);
      }
      else {
        display->current_wifi_networks_scan_page_cursor = -1;
        find_next_button = true;
        PrintLn("display->current_wifi_networks_scan_page_cursor = ", display->current_wifi_networks_scan_page_cursor);
      }
    }
    else {
      if(display->current_wifi_networks_scan_page_cursor > 0) {
        display->current_wifi_networks_scan_page_cursor--;
        PrintLn("display->current_wifi_networks_scan_page_cursor = ", display->current_wifi_networks_scan_page_cursor);
      }
      else {
        display->current_wifi_networks_scan_page_cursor = -1;
        find_next_button = true;
        PrintLn("display->current_wifi_networks_scan_page_cursor = ", display->current_wifi_networks_scan_page_cursor);
      }
    }
    // special case of kWiFiScanNetworksPage continued inside if(find_next_button)
  }
  else {
    find_next_button = true;
  }
  // find next button
  if(find_next_button) {
    if(increment) {
      if(current_cursor != display_pages_vec[current_page][last_click_button_index]->btn_cursor_id) {
        int new_button_index = DisplayPagesVecCurrentButtonIndex() + 1;
        while(display_pages_vec[current_page][new_button_index]->btn_type == kLabelOnlyNoClickButton)
          new_button_index++;
        current_cursor = display_pages_vec[current_page][new_button_index]->btn_cursor_id;
      }
      else {
        current_cursor = display_pages_vec[current_page][first_click_button_index]->btn_cursor_id;
        // special case of kWiFiScanNetworksPage continued from before if(find_next_button)
        if((current_page == kWiFiScanNetworksPage) && (current_cursor == kWiFiScanNetworksPageList)) {
          // came here from back button
          display->current_wifi_networks_scan_page_cursor = 0;
          PrintLn("display->current_wifi_networks_scan_page_cursor = ", display->current_wifi_networks_scan_page_cursor);
        }
      }
    }
    else {
      if(current_cursor != display_pages_vec[current_page][first_click_button_index]->btn_cursor_id) {
        int new_button_index = DisplayPagesVecCurrentButtonIndex() - 1;
        while(display_pages_vec[current_page][new_button_index]->btn_type == kLabelOnlyNoClickButton)
          new_button_index--;
        current_cursor = display_pages_vec[current_page][new_button_index]->btn_cursor_id;
        // special case of kWiFiScanNetworksPage continued from before if(find_next_button)
        if((current_page == kWiFiScanNetworksPage) && (current_cursor == kWiFiScanNetworksPageList)) {
          // came here from Rescan button
          display->current_wifi_networks_scan_page_cursor = display->kWifiScanNetworksPageItems - 1;
          PrintLn("display->current_wifi_networks_scan_page_cursor = ", display->current_wifi_networks_scan_page_cursor);
        }
      }
      else {
        current_cursor = display_pages_vec[current_page][last_click_button_index]->btn_cursor_id;
      }
    }
  }
  // PrintLn("current_cursor = ", current_cursor);

  display->DisplayCursorHighlight(/*highlight_On = */ true);
  // wait a little
  delay(2*kUserInputDelayMs);
}

// populate all pages in display_pages_vec
void PopulateDisplayPages() {
  //// struct decleration in common.h
  //struct DisplayButton {
  //  const Cursor btn_cursor_id;
  //  const ButtonType btn_type;
  //  const std::string row_label;
  //  const bool fixed_location;
  //  int16_t btn_x;
  //  int16_t btn_y;
  //  uint16_t btn_w;
  //  uint16_t btn_h;
  //  std::string btn_value;
  //};

  DisplayButton* page_save_button = new DisplayButton{ /* Save Button */ kPageSaveButton, kClickButtonWithLabel, "", true, kSaveButtonX1, kSaveButtonY1, kSaveButtonW, kSaveButtonH, kSaveStr };
  DisplayButton* page_back_button = new DisplayButton{ /* Back Button */ kPageBackButton, kClickButtonWithLabel, "", true, kBackButtonX1, kBackButtonY1, kBackButtonW, kBackButtonH, kBackStr };

  // MAIN PAGE
  display_pages_vec[kMainPage] = std::vector<DisplayButton*> {
    new DisplayButton{ /* No Selection */ kCursorNoSelection, kClickButtonWithIcon, "", true, -5, -5, 0, 0, "" },
    new DisplayButton{ /* Settings Wheel */ kMainPageSettingsWheel, kClickButtonWithIcon, "", true, kSettingsGearX1, kSettingsGearY1, kSettingsGearWidth, kSettingsGearHeight, "" },
    new DisplayButton{ /* Alarms Row     */ kMainPageSetAlarm, kClickButtonWithIcon, "", true, 1, kAlarmRowY1, kTftWidth - 2, kTftHeight - kAlarmRowY1 - 1, "" },
  };

  // ALARM SET PAGE
  // this page is handled as a special case

  // SETTINGS PAGE
  display_pages_vec[kSettingsPage] = std::vector<DisplayButton*> {
    new DisplayButton{ kSettingsPageWiFi, kClickButtonWithLabel, "WiFi Settings:", false, 0,0,0,0, "WIFI" },
    new DisplayButton{ kSettingsPageClock, kClickButtonWithLabel, "Clock & Time Settings:", false, 0,0,0,0, "CLOCK" },
    new DisplayButton{ kSettingsPageScreensaver, kClickButtonWithLabel, "Screensaver Settings:", false, 0,0,0,0, "SCREENSAVER" },
    new DisplayButton{ kSettingsPageRgbLed, kClickButtonWithLabel, "RGB LED Settings:", false, 0,0,0,0, "RGB LEDs" },
    new DisplayButton{ kSettingsPageWeather, kClickButtonWithLabel, "Weather Page:", false, 0,0,0,0, "WEATHER" },
    page_back_button,
  };

  // WIFI SETTINGS PAGE
  display_pages_vec[kWiFiSettingsPage] = std::vector<DisplayButton*> {
    new DisplayButton{ kWiFiSettingsPageShowSsidRow, kLabelOnlyNoClickButton, "Saved WiFi:", false, 0,0,0,0, wifi_stuff->WiFiDetailsShortString() },
    new DisplayButton{ kWiFiSettingsPageScanNetworks, kClickButtonWithLabel, "Scan Networks:", false, 0,0,0,0, "SCAN WIFI" },
    new DisplayButton{ kWiFiSettingsPageChangePasswd, kClickButtonWithLabel, "Change Password:", false, 0,0,0,0, "WIFI PASSWD" },
    new DisplayButton{ kWiFiSettingsPageClearSsidAndPasswd, kClickButtonWithLabel, "Clear WiFi Details:", false, 0,0,0,0, "CLEAR" },
    new DisplayButton{ kWiFiSettingsPageConnect, kClickButtonWithLabel, "", false, 0,0,0,0, "CONNECT WIFI" },
    new DisplayButton{ kWiFiSettingsPageDisconnect, kClickButtonWithLabel, "", false, 0,0,0,0, "DISCONNECT" },
    page_back_button,
  };

  // WIFI SCAN NETWORKS PAGE
  display_pages_vec[kWiFiScanNetworksPage] = std::vector<DisplayButton*> {
    new DisplayButton{ kWiFiScanNetworksPageList, kClickButtonWithIcon, "", true, 0, 0, 0, 0, "" },
    new DisplayButton{ kWiFiScanNetworksPageRescan, kClickButtonWithLabel, "", true, kRescanButtonX1, kRescanButtonY1, kRescanButtonW, kRescanButtonH, kRescanStr },
    new DisplayButton{ kWiFiScanNetworksPageNext, kClickButtonWithLabel, "", true, kNextButtonX1, kNextButtonY1, kNextButtonW, kNextButtonH, kNextStr },
    page_back_button,
  };

  // WIFI DETAILS SOFT AP PAGE
  display_pages_vec[kSoftApInputsPage] = std::vector<DisplayButton*> {
    page_save_button,
    page_back_button,
  };

  // CLOCK SETTINGS PAGE
  display_pages_vec[kClockSettingsPage] = std::vector<DisplayButton*> {
    new DisplayButton{ kClockSettingsPageOwnerName, kClickButtonWithLabel, "Owner:", false, 0,0,0,0, "EDIT" },
    new DisplayButton{ kClockSettingsPageSetLocation, kClickButtonWithLabel, "City:", false, 0,0,0,0, (wifi_stuff->location_zip_code_ + " " + wifi_stuff->location_country_code_) },
    new DisplayButton{ kClockSettingsPageUpdateTime, kClickButtonWithLabel, "Update Time:", false, 0,0,0,0, "UPDATE TIME" },
    new DisplayButton{ kClockSettingsPageAlarmLongPressTime, kClickButtonWithLabel, "Long Press Alarm Time:", false, 0,0,0,0, (std::to_string(alarm_clock->alarm_long_press_seconds_) + "sec") },
    new DisplayButton{ kClockSettingsPageDisplaySettings, kClickButtonWithLabel, "Display Settings:", false, 0,0,0,0, "DISPLAY" },
    new DisplayButton{ kClockSettingsPageUpdateFirmware, kClickButtonWithLabel, "Firmware Update:", false, 0,0,0,0, "UPDATE" },
    page_back_button,
  };

  // DISPLAY SETTINGS PAGE
  display_pages_vec[kDisplaySettingsPage] = std::vector<DisplayButton*> {
    new DisplayButton{ kDisplaySettingsPageRotateScreen, kClickButtonWithLabel, "Rotate Screen:", false, 0,0,0,0, "ROTATE" },
  };
  if(ts != nullptr) {
    display_pages_vec[kDisplaySettingsPage].push_back(new DisplayButton{ kDisplaySettingsPageTestTouchscreenCalibration, kClickButtonWithLabel, "Touchscreen Calibration", false, 0,0,0,0, "TEST" });
    display_pages_vec[kDisplaySettingsPage].push_back(new DisplayButton{ kDisplaySettingsPageCalibrateTouchscreen, kClickButtonWithLabel, "Touchscreen", false, 0,0,0,0, "RE-CALIBRATION" });
  }
  display_pages_vec[kDisplaySettingsPage].push_back(page_back_button);

  // WEATHER SETTINGS PAGE
  display_pages_vec[kWeatherSettingsPage] = std::vector<DisplayButton*> {
    new DisplayButton{ kWeatherSettingsPageSetUnits, kClickButtonWithLabel, "Set Units:", false, 0,0,0,0, (wifi_stuff->weather_units_metric_not_imperial_ ? kMetricUnitStr : kImperialUnitStr) },
    new DisplayButton{ kWeatherSettingsPageFetch, kClickButtonWithLabel, "Fetch Weather:", false, 0,0,0,0, "FETCH" },
    page_back_button,
  };

  // LOCATION INPUT LOCAL SERVER PAGE
  display_pages_vec[kLocationInputsLocalServerPage] = std::vector<DisplayButton*> {
    page_save_button,
    page_back_button,
  };

  // SCREENSAVER SETTINGS PAGE
  display_pages_vec[kScreensaverSettingsPage] = std::vector<DisplayButton*> {
    new DisplayButton{ kScreensaverSettingsPageMotion, kClickButtonWithLabel, "Screensaver Motion:", false, 0,0,0,0, (display->screensaver_bounce_not_fly_horizontally_ ? kBounceScreensaverStr : kFlyOutScreensaverStr) },
    new DisplayButton{ kScreensaverSettingsPageSpeed, kClickButtonWithLabel, "Screensaver Speed:", false, 0,0,0,0, (cpu_speed_mhz == 80 ? kSlowStr : (cpu_speed_mhz == 160 ? kMediumStr : kFastStr)) },
    new DisplayButton{ kScreensaverSettingsPageRun, kClickButtonWithLabel, "Run Screensaver:", false, 0,0,0,0, "RUN" },
    new DisplayButton{ kScreensaverSettingsPageNightTimeColorChange, kClickButtonWithLabel, "Sleep-friendly color at night:", false, 0,0,0,0, (display->sleep_friendly_color_at_night ? kYesStr : kNoStr) },
    new DisplayButton{ kScreensaverSettingsPageMinBrightness, kClickButtonWithLabel, "Minimum Brightness:", false, 0,0,0,0, ("  " + std::to_string(display->min_brightness_adder) + "  ") },
    page_back_button,
  };

  // RGB LED SETTINGS PAGE
  display_pages_vec[kRgbLedSettingsPage] = std::vector<DisplayButton*> {
    new DisplayButton{ kRgbLedSettingsPageRgbLedStripMode, kClickButtonWithLabel, "RGB LEDs Mode:", false, 0,0,0,0, RgbLedSettingString() },
    new DisplayButton{ kRgbLedSettingsPageNightTmDimHr, kClickButtonWithLabel, ("Evening time is " + std::to_string(kEveningTimeMinutes / 60 - 12) + "PM to:"), false, 0,0,0,0, (std::to_string(nvs_preferences->RetrieveNightTimeDimHour()) + "PM") },
    new DisplayButton{ kRgbLedSettingsPageRgbLedBrightness, kClickButtonWithLabel, "RGB LEDs Brightness:", false, 0,0,0,0, (std::to_string(int(static_cast<float>(rgb_strip_led_brightness) / 255 * 100)) + "%") },
    page_back_button,
  };

}

int DisplayPagesVecCurrentButtonIndex() {
  for (int i = 0; i < display_pages_vec[current_page].size(); i++) {
    if(display_pages_vec[current_page][i]->btn_cursor_id == current_cursor)
      return i;
  }
  return -1;
}

int DisplayPagesVecButtonIndex(ScreenPage button_page, Cursor button_cursor) {
  for (int i = 0; i < display_pages_vec[button_page].size(); i++) {
    if(display_pages_vec[button_page][i]->btn_cursor_id == button_cursor)
      return i;
  }
  return -1;
}

void LedFeedbackOnOff() {
  LedFeedback(HIGH);
  delay(kUserInputDelayMs);
  LedFeedback(LOW);
}

void LedFeedback(bool value) {
  digitalWrite(LED_PIN(), value);
}

// takes in WiFi Password input using Touchscreen or by creating a SoftAP
void WiFiPasswordInputTouchAndNonTouch() {
  LedFeedback(true);
  // get WiFi Password Input
  if(ts != NULL) {
    // use touchscreen
    std::string wifi_pwd_str = "";
    // user input string
    std::string label = "Enter Password for WiFi:\n" + wifi_stuff->wifi_ssid_;
    char returnText[kWifiSsidPasswordLengthMax + 1] = "";
    // for(int i = 0; i< wifi_stuff->wifi_password_.size(); i++) {
    //   returnText[i] = wifi_stuff->wifi_password_[i];
    // }
    LedFeedback(false);
    // get WiFi Password input from screen
    bool ret = display->GetUserOnScreenTextInput(label, returnText, /* bool numbers_only = */ false, /* bool alphabets_only = */ false);
    PrintLn(returnText);
    if(ret) {
      LedFeedback(true);
      wifi_stuff->wifi_password_ = returnText;
      wifi_stuff->SaveWiFiDetails();
    }
    SetPage(kWiFiSettingsPage);
  }
  else {
    // start a SoftAP and take user input
    AddSecondCoreTaskIfNotThere(kStartSetWiFiSoftAP);
    WaitForExecutionOfSecondCoreTask();
    SetPage(kSoftApInputsPage);
  }
}

/*  ButtonClickUiFeedback(kTurnOn_Delay_TurnOff);  (default)
    ButtonClickUiFeedback(kTurnOn_Delay);
    ButtonClickUiFeedback(kTurnOn);
    ButtonClickUiFeedback(kTurnOff);   */
void ButtonClickUiFeedback(ButtonClickUiFeedbackType response_type = kTurnOn_Delay_TurnOff) {
  switch (response_type) {
    case kTurnOn_Delay:   // turn On Button, delay a little
      display->DisplayCurrentPageButtonRow(/*is_on = */ true);
      delay(kUserInputDelayMs);
      break;
    case kTurnOn:   // turn On Button
      display->DisplayCurrentPageButtonRow(/*is_on = */ true);
      break;
    case kTurnOff:   // turn Off Button
      display->DisplayCurrentPageButtonRow(/*is_on = */ false);
      break;
    case kTurnOn_Delay_TurnOff:
    default:     // turn On Button, delay, turn Off button
      display->DisplayCurrentPageButtonRow(/*is_on = */ true);
      delay(kUserInputDelayMs);
      display->DisplayCurrentPageButtonRow(/*is_on = */ false);
  }
}

void ButtonClickAction() {
  if(current_page == kAlarmSetPage)
    display->SetAlarmScreen(/* process_user_input */ true, /* inc_button_pressed */ false, /* dec_button_pressed */ false, /* push_button_pressed */ true);
  else {
    if(current_page == kMainPage) {                 // MAIN PAGE
      if(current_cursor == kMainPageSettingsWheel) {
        ButtonClickUiFeedback(kTurnOn_Delay);
        SetPage(kSettingsPage);
      }
      else if(current_cursor == kMainPageSetAlarm) {
        ButtonClickUiFeedback(kTurnOn_Delay);
        SetPage(kAlarmSetPage);
      }
    }
    else if(current_page == kSettingsPage) {        // SETTINGS PAGE
      if(current_cursor == kSettingsPageWiFi) {
        ButtonClickUiFeedback(kTurnOn);
        LedFeedback(true);
        SetPage(kWiFiSettingsPage);
      }
      else if(current_cursor == kSettingsPageClock) {
        ButtonClickUiFeedback(kTurnOn_Delay);
        SetPage(kClockSettingsPage);
      }
      else if(current_cursor == kSettingsPageScreensaver) {
        ButtonClickUiFeedback(kTurnOn_Delay);
        SetPage(kScreensaverSettingsPage);
      }
      else if(current_cursor == kSettingsPageRgbLed) {
        ButtonClickUiFeedback(kTurnOn_Delay);
        SetPage(kRgbLedSettingsPage);
      }
      else if(current_cursor == kSettingsPageWeather) {
        ButtonClickUiFeedback(kTurnOn);
        LedFeedback(true);
        AddSecondCoreTaskIfNotThere(kGetWeatherInfo);
        WaitForExecutionOfSecondCoreTask();
        SetPage(kWeatherSettingsPage);
      }
      else if(current_cursor == kPageBackButton) {
        ButtonClickUiFeedback(kTurnOn_Delay);
        SetPage(kMainPage);
      }
    }
    else if(current_page == kWiFiSettingsPage) {          // WIFI SETTINGS PAGE
      if(current_cursor == kWiFiSettingsPageScanNetworks) {
        ButtonClickUiFeedback(kTurnOn);
        LedFeedback(true);
        AddSecondCoreTaskIfNotThere(kScanNetworks);
        WaitForExecutionOfSecondCoreTask();
        SetPage(kWiFiScanNetworksPage);
      }
      else if(current_cursor == kWiFiSettingsPageChangePasswd) {
        ButtonClickUiFeedback(kTurnOn_Delay);
        LedFeedback(true);
        WiFiPasswordInputTouchAndNonTouch();
      }
      else if(current_cursor == kWiFiSettingsPageClearSsidAndPasswd) {
        ButtonClickUiFeedback(kTurnOn);
        LedFeedbackOnOff();
        AddSecondCoreTaskIfNotThere(kDisconnectWiFi);
        WaitForExecutionOfSecondCoreTask();
        wifi_stuff->wifi_ssid_ = "Scan WiFi";
        wifi_stuff->wifi_password_ = "Enter Passwd";
        wifi_stuff->SaveWiFiDetails();
        // update Settings Page WiFi ssid row
        int index_of_ssid_button = DisplayPagesVecButtonIndex(kWiFiSettingsPage, kWiFiSettingsPageShowSsidRow);
        display_pages_vec[kWiFiSettingsPage][index_of_ssid_button]->btn_value = wifi_stuff->WiFiDetailsShortString();
        SetPage(kWiFiSettingsPage, /* bool move_cursor_to_first_button = */ false);
      }
      else if(current_cursor == kWiFiSettingsPageConnect) {
        ButtonClickUiFeedback(kTurnOn);
        LedFeedback(true);
        AddSecondCoreTaskIfNotThere(kConnectWiFi);
        WaitForExecutionOfSecondCoreTask();
        ButtonClickUiFeedback(kTurnOff);
        // LedFeedback(false);
        // display->DisplayWiFiConnectionStatus();
        display->DisplayCurrentPage();
        // delay(kUserInputDelayMs);
        display->DisplayCursorHighlight(/*highlight_On = */ true);
        LedFeedback(false);
      }
      else if(current_cursor == kWiFiSettingsPageDisconnect) {
        LedFeedback(true);
        ButtonClickUiFeedback(kTurnOn_Delay);
        AddSecondCoreTaskIfNotThere(kDisconnectWiFi);
        WaitForExecutionOfSecondCoreTask();
        ButtonClickUiFeedback(kTurnOff);
        // display->DisplayWiFiConnectionStatus();
        // LedFeedback(false);
        display->DisplayCurrentPage();
        delay(kUserInputDelayMs);
        display->DisplayCursorHighlight(/*highlight_On = */ true);
        LedFeedback(false);
      }
      else if(current_cursor == kPageBackButton) {
        ButtonClickUiFeedback(kTurnOn_Delay);
        current_cursor = kSettingsPageWiFi;
        SetPage(kSettingsPage);
      }
    }
    else if(current_page == kClockSettingsPage) {           // CLOCK SETTINGS PAGE
      if(current_cursor == kClockSettingsPageOwnerName) {
        ButtonClickUiFeedback(kTurnOn_Delay);
        if(ts != NULL) {
          // use touchscreen
          std::string label = "Enter your name:";
          std::string owner_name;
          nvs_preferences->RetrieveOwnerName(owner_name);
          char returnText[kWifiSsidPasswordLengthMax + 1] = "";
          for(int i = 0; i< owner_name.size(); i++) {
            returnText[i] = owner_name[i];
          }
          // get User Name input from screen
          bool ret = display->GetUserOnScreenTextInput(label, returnText, /* bool numbers_only = */ false, /* bool alphabets_only = */ false);
          PrintLn(returnText);
          if(ret) {
            display->DisplayBlankScreen();
            LedFeedbackOnOff();
            std::string owner_name_str = returnText;
            nvs_preferences->SaveOwnerName(owner_name_str);
          }
          SetPage(kClockSettingsPage);
        }
        else {
          AddSecondCoreTaskIfNotThere(kStartLocationInputsLocalServer);
          WaitForExecutionOfSecondCoreTask();
          SetPage(kLocationInputsLocalServerPage);
        }
      }
      else if(current_cursor == kClockSettingsPageSetLocation) {
        LedFeedback(true);
        ButtonClickUiFeedback(kTurnOn_Delay);
        if(ts != NULL) {
          // use touchscreen
          std::string label = "Enter the 5-digit ZIP or 6-\ndigit PIN of your city:";
          std::string location_zip_code = wifi_stuff->location_zip_code_;
          char returnText[kWifiSsidPasswordLengthMax + 1] = "";
          for(int i = 0; i< location_zip_code.size(); i++) {
            returnText[i] = location_zip_code[i];
          }
          LedFeedback(false);
          // get ZIP Code input from screen
          bool ret = display->GetUserOnScreenTextInput(label, returnText, /* bool numbers_only = */ true, /* bool alphabets_only = */ false);
          PrintLn(returnText);
          if(ret) {
            display->DisplayBlankScreen();
            LedFeedbackOnOff();
            std::string new_location_zip_str = returnText;
            wifi_stuff->location_zip_code_ = new_location_zip_str;

            // get Country Code

            label = "Enter your Country's 2-letter\nCountry Code or Initials:";
            strcpy(returnText, "");
            strcpy(returnText, wifi_stuff->location_country_code_.c_str());
            // get 2-letter Country Code input from screen
            bool ret = display->GetUserOnScreenTextInput(label, returnText, /* bool numbers_only = */ false, /* bool alphabets_only = */ true);
            PrintLn(returnText);
            if(ret) {
              LedFeedback(true);
              display->DisplayBlankScreen();
              wifi_stuff->location_country_code_ = returnText;
              // update new location Zip/Pin code on button
              int display_pages_vec_location_button_index = DisplayPagesVecButtonIndex(kClockSettingsPage, kClockSettingsPageSetLocation);
              std::string location_str = (wifi_stuff->location_zip_code_ + " " + wifi_stuff->location_country_code_);
              display_pages_vec[kClockSettingsPage][display_pages_vec_location_button_index]->btn_value = location_str;
            }

            // save new location details
            wifi_stuff->SaveNewLocationAndClearCity();

            // get new location, update time and weather info
            AddSecondCoreTaskIfNotThere(kUpdateTimeFromNtpServer);
            WaitForExecutionOfSecondCoreTask();
          }
          SetPage(kClockSettingsPage);
        }
        else {
          AddSecondCoreTaskIfNotThere(kStartLocationInputsLocalServer);
          WaitForExecutionOfSecondCoreTask();
          SetPage(kLocationInputsLocalServerPage);
        }
      }
      else if(current_cursor == kClockSettingsPageUpdateTime) {
        LedFeedback(true);
        ButtonClickUiFeedback(kTurnOn);
        AddSecondCoreTaskIfNotThere(kUpdateTimeFromNtpServer);
        WaitForExecutionOfSecondCoreTask();
        if(wifi_stuff->manual_time_update_successful_)
          SetPage(kMainPage);
        else
          SetPage(kClockSettingsPage, /* bool move_cursor_to_first_button = */ false);
      }
      else if(current_cursor == kClockSettingsPageAlarmLongPressTime) {
        // change seconds
        if(alarm_clock->alarm_long_press_seconds_ < 25)
          alarm_clock->alarm_long_press_seconds_ += 10;
        else
          alarm_clock->alarm_long_press_seconds_ = 5;
        display_pages_vec[current_page][DisplayPagesVecCurrentButtonIndex()]->btn_value = std::to_string(alarm_clock->alarm_long_press_seconds_) + "sec";
        nvs_preferences->SaveLongPressSeconds(alarm_clock->alarm_long_press_seconds_);
        ButtonClickUiFeedback(kTurnOn_Delay_TurnOff);
      }
      else if(current_cursor == kClockSettingsPageDisplaySettings) {
        ButtonClickUiFeedback(kTurnOn_Delay);
        SetPage(kDisplaySettingsPage);
      }
      else if(current_cursor == kClockSettingsPageUpdateFirmware) {
        LedFeedback(true);
        ButtonClickUiFeedback(kTurnOn);
        AddSecondCoreTaskIfNotThere(kFirmwareVersionCheck);
        WaitForExecutionOfSecondCoreTask();
        if(wifi_stuff->firmware_update_available_str_.size() > 0)
          display->DisplayFirmwareVersionAndDate();
        ButtonClickUiFeedback(kTurnOff);
        LedFeedback(false);
      }
      else if(current_cursor == kPageBackButton) {
        ButtonClickUiFeedback(kTurnOn_Delay);
        current_cursor = kSettingsPageClock;
        SetPage(kSettingsPage, /* bool move_cursor_to_first_button = */ false);
      }
      
    }
    else if(current_page == kDisplaySettingsPage) {        // DISPLAY SETTINGS PAGE
      if(current_cursor == kDisplaySettingsPageRotateScreen) {
        ButtonClickUiFeedback(kTurnOn);
        LedFeedbackOnOff();
        // rotate screen 180 degrees
        display->RotateScreen();
        if(ts != NULL)
          ts->SetTouchscreenOrientation();
        SetPage(kDisplaySettingsPage, /* bool move_cursor_to_first_button = */ false);
        ButtonClickUiFeedback(kTurnOff);
      }
      else if(current_cursor == kDisplaySettingsPageTestTouchscreenCalibration) {
        ButtonClickUiFeedback(kTurnOn_Delay);
        TestTouchscreenCalibrationFn();
        // set back current page
        SetPage(current_page);
        inactivity_millis = 0;
      }
      else if(current_cursor == kDisplaySettingsPageCalibrateTouchscreen) {
        ButtonClickUiFeedback(kTurnOn_Delay);
        CalibrateTouchscreenFn();
        // set back current page
        SetPage(current_page);
        inactivity_millis = 0;
      }
      else if(current_cursor == kPageBackButton) {
        ButtonClickUiFeedback(kTurnOn_Delay);
        SetPage(kClockSettingsPage);
      }
    }
    else if(current_page == kWiFiScanNetworksPage) {          // WIFI NETWORKS SCAN PAGE
      if(current_cursor == kWiFiScanNetworksPageList) {
        ButtonClickUiFeedback(kTurnOn_Delay);
        LedFeedback(true);
        delay(kUserInputDelayMs); // extra delay
        int index_of_selected_ssid = display->current_wifi_networks_scan_page_cursor + display->current_wifi_networks_scan_page_no * display->kWifiScanNetworksPageItems;
        PrintLn("index_of_selected_ssid = ", index_of_selected_ssid);
        if((index_of_selected_ssid > wifi_stuff->WiFiScanNetworksCount() - 1) || (index_of_selected_ssid < 0))
          return;
        // save selected WiFi SSID
        wifi_stuff->wifi_ssid_ = wifi_stuff->WiFiScanNetworkSsid(index_of_selected_ssid);
        wifi_stuff->WiFiScanNetworksFreeMemory();
        wifi_stuff->SaveWiFiDetails();
        // update Settings Page WiFi ssid row
        int index_of_ssid_button = DisplayPagesVecButtonIndex(kWiFiSettingsPage, kWiFiSettingsPageShowSsidRow);
        display_pages_vec[kWiFiSettingsPage][index_of_ssid_button]->btn_value = wifi_stuff->WiFiDetailsShortString();
        // get WiFi Password Input
        WiFiPasswordInputTouchAndNonTouch();
      }
      if(current_cursor == kWiFiScanNetworksPageRescan) {
        LedFeedback(true);
        ButtonClickUiFeedback(kTurnOn);
        AddSecondCoreTaskIfNotThere(kScanNetworks);
        WaitForExecutionOfSecondCoreTask();
        SetPage(kWiFiScanNetworksPage);
      }
      else if(current_cursor == kWiFiScanNetworksPageNext) {
        ButtonClickUiFeedback(kTurnOn_Delay_TurnOff);
        SetPage(kWiFiScanNetworksPage, false, true);
      }
      else if(current_cursor == kPageBackButton) {
        ButtonClickUiFeedback(kTurnOn_Delay);
        wifi_stuff->WiFiScanNetworksFreeMemory();
        SetPage(kWiFiSettingsPage);
      }
    }
    else if(current_page == kSoftApInputsPage) {          // SOFT AP SET WIFI SSID PASSWD PAGE
      if(current_cursor == kPageSaveButton) {
        ButtonClickUiFeedback(kTurnOn);
        LedFeedback(true);
        wifi_stuff->save_SAP_details_ = true;
      }
      else if(current_cursor == kPageBackButton) {
        ButtonClickUiFeedback(kTurnOn_Delay);
        // don't save wifi details
      }
      AddSecondCoreTaskIfNotThere(kStopSetWiFiSoftAP);
      WaitForExecutionOfSecondCoreTask();
      SetPage(kWiFiSettingsPage);
    }
    else if(current_page == kWeatherSettingsPage) {       // WEATHER SETTINGS PAGE
      if(current_cursor == kWeatherSettingsPageSetUnits) {
        ButtonClickUiFeedback(kTurnOn);
        LedFeedback(true);
        wifi_stuff->weather_units_metric_not_imperial_ = !wifi_stuff->weather_units_metric_not_imperial_;
        wifi_stuff->SaveNewWeatherUnits();
        display_pages_vec[current_page][DisplayPagesVecCurrentButtonIndex()]->btn_value = (wifi_stuff->weather_units_metric_not_imperial_ ? kMetricUnitStr : kImperialUnitStr);
        // fetch weather info in new units
        AddSecondCoreTaskIfNotThere(kGetWeatherInfo);
        WaitForExecutionOfSecondCoreTask();
        SetPage(kWeatherSettingsPage, /* bool move_cursor_to_first_button = */ false);
      }
      else if(current_cursor == kWeatherSettingsPageFetch) {
        ButtonClickUiFeedback(kTurnOn);
        LedFeedback(true);
        AddSecondCoreTaskIfNotThere(kGetWeatherInfo);
        WaitForExecutionOfSecondCoreTask();
        SetPage(kWeatherSettingsPage, /* bool move_cursor_to_first_button = */ false);
      }
      else if(current_cursor == kPageBackButton) {
        ButtonClickUiFeedback(kTurnOn_Delay);
        current_cursor = kSettingsPageWeather;
        SetPage(kSettingsPage, /* bool move_cursor_to_first_button = */ false);
      }
    }
    else if(current_page == kLocationInputsLocalServerPage) {          // LOCATION INPUT LOCAL SERVER PAGE
      if(current_cursor == kPageSaveButton) {
        ButtonClickUiFeedback(kTurnOn);
        LedFeedback(true);
        wifi_stuff->save_SAP_details_ = true;
        AddSecondCoreTaskIfNotThere(kStopLocationInputsLocalServer);
        WaitForExecutionOfSecondCoreTask();
        wifi_stuff->got_weather_info_ = false;
        wifi_stuff->city_ = "";
        // update new location Zip/Pin code on button
        int display_pages_vec_location_button_index = DisplayPagesVecButtonIndex(kClockSettingsPage, kClockSettingsPageSetLocation);
        std::string location_str = (wifi_stuff->location_zip_code_ + " " + wifi_stuff->location_country_code_);
        display_pages_vec[kClockSettingsPage][display_pages_vec_location_button_index]->btn_value = location_str;
        // get new location, update time and weather info
        AddSecondCoreTaskIfNotThere(kUpdateTimeFromNtpServer);
      }
      else if(current_cursor == kPageBackButton) {
        ButtonClickUiFeedback(kTurnOn_Delay);
        AddSecondCoreTaskIfNotThere(kStopLocationInputsLocalServer);
      }
      WaitForExecutionOfSecondCoreTask();
      SetPage(kClockSettingsPage);
    }
    else if(current_page == kScreensaverSettingsPage) {        // SCREENSAVER SETTINGS PAGE
      if(current_cursor == kScreensaverSettingsPageMotion) {
        display->screensaver_bounce_not_fly_horizontally_ = !display->screensaver_bounce_not_fly_horizontally_;
        display_pages_vec[current_page][DisplayPagesVecCurrentButtonIndex()]->btn_value = (display->screensaver_bounce_not_fly_horizontally_ ? kBounceScreensaverStr : kFlyOutScreensaverStr);
        nvs_preferences->SaveScreensaverBounceNotFlyHorizontally(display->screensaver_bounce_not_fly_horizontally_);
        ButtonClickUiFeedback(kTurnOn_Delay_TurnOff);
      }
      else if(current_cursor == kScreensaverSettingsPageSpeed) {
        CycleCpuFrequency();
        display_pages_vec[current_page][DisplayPagesVecCurrentButtonIndex()]->btn_value = (cpu_speed_mhz == 80 ? kSlowStr : (cpu_speed_mhz == 160 ? kMediumStr : kFastStr));
        ButtonClickUiFeedback(kTurnOn_Delay_TurnOff);
      }
      else if(current_cursor == kScreensaverSettingsPageRun) {
        ButtonClickUiFeedback(kTurnOn_Delay);
        SetPage(kScreensaverPage);
      }
      else if(current_cursor == kScreensaverSettingsPageNightTimeColorChange) {
        display->sleep_friendly_color_at_night = !display->sleep_friendly_color_at_night;
        nvs_preferences->SaveScreensaverSleepFriendNightColor(display->sleep_friendly_color_at_night);
        display_pages_vec[current_page][DisplayPagesVecCurrentButtonIndex()]->btn_value = (display->sleep_friendly_color_at_night ? kYesStr : kNoStr);
        ButtonClickUiFeedback(kTurnOn_Delay_TurnOff);
      }
      else if(current_cursor == kScreensaverSettingsPageMinBrightness) {
        ButtonClickUiFeedback(kTurnOn_Delay);
        // change brightness
        if(display->min_brightness_adder < 4)
          display->min_brightness_adder++;
        else if(display->min_brightness_adder == 4)
          display->min_brightness_adder = 7;
        else if(display->min_brightness_adder == 7)
          display->min_brightness_adder = 10;
        else if(display->min_brightness_adder == 10)
          display->min_brightness_adder = 15;
        else if(display->min_brightness_adder == 15)
          display->min_brightness_adder = 20;
        else
          display->min_brightness_adder = 0;
        display_pages_vec[current_page][DisplayPagesVecCurrentButtonIndex()]->btn_value = ("  " + std::to_string(display->min_brightness_adder) + "  ");
        nvs_preferences->SaveDisplayMinBrightnessAdder(display->min_brightness_adder);
        display->SetBrightness(display->min_brightness_adder + display->kNightBrightness);
        ButtonClickUiFeedback(kTurnOff);
      }
      else if(current_cursor == kPageBackButton) {
        ButtonClickUiFeedback(kTurnOn_Delay);
        current_cursor = kSettingsPageScreensaver;
        SetPage(kSettingsPage, /* bool move_cursor_to_first_button = */ false);
      }
    }
    else if(current_page == kRgbLedSettingsPage) {        // RGB LED SETTINGS PAGE
      if(current_cursor == kRgbLedSettingsPageNightTmDimHr) {
        // change hours
        uint8_t night_time_dim_hour = nvs_preferences->RetrieveNightTimeDimHour();
        if(night_time_dim_hour < 11)
          night_time_dim_hour++;
        else
          night_time_dim_hour = 7;
        nvs_preferences->SaveNightTimeDimHour(night_time_dim_hour);
        night_time_minutes = night_time_dim_hour * 60 + 720;
        display_pages_vec[current_page][DisplayPagesVecCurrentButtonIndex()]->btn_value = (std::to_string(night_time_dim_hour) + "PM");
        ButtonClickUiFeedback(kTurnOn_Delay_TurnOff);
      }
      else if(current_cursor == kRgbLedSettingsPageRgbLedStripMode) {
        if((uint8_t)autorun_rgb_led_strip_mode < 3)
          autorun_rgb_led_strip_mode = (RgbLedMode)((uint8_t)autorun_rgb_led_strip_mode + 1);
        else
          autorun_rgb_led_strip_mode = RGB_LED_MANUAL_OFF;
        nvs_preferences->SaveAutorunRgbLedStripMode((uint8_t)autorun_rgb_led_strip_mode);
        RunRgbLedAccordingToSettings();
        display_pages_vec[current_page][DisplayPagesVecCurrentButtonIndex()]->btn_value = RgbLedSettingString();
        ButtonClickUiFeedback(kTurnOn_Delay_TurnOff);
      }
      else if(current_cursor == kRgbLedSettingsPageRgbLedBrightness) {
        ButtonClickUiFeedback(kTurnOn_Delay);
        // change brightness
        if(rgb_strip_led_brightness < 255)
          rgb_strip_led_brightness += 51;
        else
          rgb_strip_led_brightness = 51;
        display_pages_vec[current_page][DisplayPagesVecCurrentButtonIndex()]->btn_value = (std::to_string(int(static_cast<float>(rgb_strip_led_brightness) / 255 * 100)) + "%");
        nvs_preferences->SaveRgbStripLedBrightness(rgb_strip_led_brightness);
        InitializeRgbLed();
        RunRgbLedAccordingToSettings();
        ButtonClickUiFeedback(kTurnOff);
      }
      else if(current_cursor == kPageBackButton) {
        ButtonClickUiFeedback(kTurnOn_Delay);
        current_cursor = kSettingsPageScreensaver;
        SetPage(kSettingsPage, /* bool move_cursor_to_first_button = */ false);
      }
      
    }
  }
}


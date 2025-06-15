#include <string>
#include "rgb_display.h"
#include "alarm_clock.h"
#include "rtc.h"
#include "nvs_preferences.h"

void RGBDisplay::Setup() {

  /* INITIALIZE DISPLAYS */

  // tft display backlight control PWM output pin
  pinMode(TFT_BL, OUTPUT);

#if defined(DISPLAY_IS_ST7789V)

  // OR use this initializer (uncomment) if using a 2.0" 320x240 TFT:
  // tft.init(TFT_HEIGHT, TFT_WIDTH);           // Init ST7789 320x240
  uint32_t SPI_Speed = 80000000;
  tft.init(kTftHeight, kTftWidth);           // Init ST7789 320x240
  tft.setSPISpeed(SPI_Speed);
  tft.invertDisplay(false);
  screen_orientation_ = nvs_preferences->RetrieveScreenOrientation();
  tft.setRotation(screen_orientation_);

#elif defined(DISPLAY_IS_ST7796)

  tft.init();           // Init ST7796 480x320
  // make display landscape orientation
  tft.setRotation(1);
  screen_orientation_ = nvs_preferences->RetrieveScreenOrientation();
  tft.setRotation(screen_orientation_);

#elif defined(DISPLAY_IS_ST7735)

  // Use this initializer if using a 1.8" ST7735 TFT screen:
  tft.initR(INITR_BLACKTAB);  // Init ST7735 chip, black tab
  // set col and row offset of display for ST7735S
  tft.setColRowStart(2, 1);
  // make display landscape orientation
  tft.setRotation(1);

#elif defined(DISPLAY_IS_ILI9341)

  // Use this initializer if using a 1.8" ILI9341 TFT screen:
  tft.begin();
  // make display landscape orientation
  tft.setRotation(1);

#elif defined(DISPLAY_IS_ILI9488)

  tft.begin();
  tft.setRotation(tft.getRotation() + 2);
  int16_t x, y;
  tft.getOrigin(&x, &y);
  x = 0, y = 0;
  tft.setOrigin(x, y);
  tft.setClipRect();
  tft.setFontAdafruit();
  tft.invertDisplay(true);

#endif

  // SPI speed defaults to SPI_DEFAULT_FREQ defined in the library, you can override it here
  // Note that speed allowable depends on chip and quality of wiring, if you go too fast, you
  // may end up with a black screen some times, or all the time.
  // tft.setSPISpeed(80000000);

  // clear screen
  tft.fillScreen(kDisplayBackroundColor);
  tft.setTextWrap(false);

  // update TFT display
  DisplayTimeUpdate();

  // fetch night time dim hour
  night_time_minutes = nvs_preferences->RetrieveNightTimeDimHour() * 60 + 720;
  #ifdef MORE_LOGS
  PrintLn("night_time_minutes", night_time_minutes);
  PrintLn("use_photoresistor", use_photoresistor);
  #endif

  // fetch display min brightness adder
  min_brightness_adder = nvs_preferences->RetrieveDisplayMinBrightnessAdder();

  if(use_photoresistor) {
    // configure Photoresistor pin
    pinMode(PHOTORESISTOR_PIN(), INPUT);
    analogReadResolution(kAdcResolutionBits);
  }
  ReAdjustDisplayBrightness();

  // set screensaver motion
  screensaver_bounce_not_fly_horizontally_ = nvs_preferences->RetrieveScreensaverBounceNotFlyHorizontally();

  sleep_friendly_color_at_night = nvs_preferences->RetrieveScreensaverSleepFriendNightColor();

  PrintLn("Display", kInitializedStr);
}

void RGBDisplay::RotateScreen() {
  if(screen_orientation_ == 1)
    screen_orientation_ = 3;
  else
    screen_orientation_ = 1;
  nvs_preferences->SaveScreenOrientation(screen_orientation_);
  tft.setRotation(screen_orientation_);
}

// set display brightness function
void RGBDisplay::SetBrightness(int brightness) {
  if(current_brightness_ != brightness) {
    analogWrite(TFT_BL, brightness);
    current_brightness_ = brightness;
    #ifdef MORE_LOGS
      PrintLn("Display Brightness set to ", brightness);
    #endif
  }
  if(use_photoresistor) {
    // hysteresis in background color on / off
    if(!show_colored_edge_screensaver_ && brightness >= kBrightnessBackgroundColorThreshold + 5 + min_brightness_adder)
      show_colored_edge_screensaver_ = true;
    else if(show_colored_edge_screensaver_ && brightness <= kBrightnessBackgroundColorThreshold - 5 + min_brightness_adder)
      show_colored_edge_screensaver_ = false;
  }
  else
    show_colored_edge_screensaver_ = (brightness >= kEveningBrightness);
}

void RGBDisplay::SetMaxBrightness() {
  inactivity_millis = 0;
  if(current_brightness_ != kMaxBrightness)
    SetBrightness(kMaxBrightness);
}

void RGBDisplay::CheckPhotoresistorAndSetBrightness() {
  int photodiode_light_raw = analogRead(PHOTORESISTOR_PIN());
  // int lcd_brightness_val = max(photodiode_light_raw * kBrightnessInactiveMax / kPhotodiodeLightRawMax, 1);
  int lcd_brightness_val2 = max((int)map(photodiode_light_raw, 0.2 / 3.3 * kPhotodiodeLightRawMax, kPhotodiodeLightRawMax, (kNightBrightness + min_brightness_adder), kBrightnessInactiveMax), (kNightBrightness + min_brightness_adder));
  if(rgb_led_strip_on)
    lcd_brightness_val2 = max(lcd_brightness_val2, kRgbStripOnDispMinBrightness + min_brightness_adder);
  else if(rtc->todays_minutes < night_time_minutes && rtc->todays_minutes >= kDayTimeMinutes)
    lcd_brightness_val2 = max(lcd_brightness_val2, kNonNightMinBrightness + min_brightness_adder);
  #ifdef MORE_LOGS
  if(debug_mode)
    Serial.print("photodiode_light_raw="); Serial.print(photodiode_light_raw); Serial.print(", lcd_brightness_val2="); Serial.println(lcd_brightness_val2);
  #endif
  SetBrightness(lcd_brightness_val2);
}

void RGBDisplay::CheckTimeAndSetBrightness() {
  // check if RTC is good
  if(rtc->year() < 2024) {
    // RTC Time is not set!
    // Keep screen on day brightness
    SetBrightness(kDayBrightness);
  }
  else {
    if(rtc->todays_minutes >= night_time_minutes)
      SetBrightness(kNightBrightness + min_brightness_adder);
    else if(rtc->todays_minutes >= kEveningTimeMinutes)
      SetBrightness(kEveningBrightness);
    else if(rtc->todays_minutes >= kDayTimeMinutes)
      SetBrightness(kDayBrightness);
    else
      SetBrightness(kNightBrightness + min_brightness_adder);
  }
}

void RGBDisplay::ReAdjustDisplayBrightness() {
  if(use_photoresistor)
    // check photoresistor brightness and adjust display brightness
    CheckPhotoresistorAndSetBrightness();
  else
    // set display brightness based on time
    CheckTimeAndSetBrightness();
}

void RGBDisplay::ScreensaverControl(bool turnOn) {
  if(!turnOn && my_canvas_ != NULL) {
    // delete screensaverCanvas;
    delete my_canvas_;
    my_canvas_ = NULL;
  }
  else
    refresh_screensaver_canvas_ = true;
  // clear screen
  tft.fillScreen(kDisplayBackroundColor);
  screensaver_x1_ = 0;
  screensaver_y1_ = 20;
  if(!screensaver_bounce_not_fly_horizontally_)   // if fly horizontally
    screensaver_move_right_ = true;
  redraw_display_ = true;
  PrepareTimeDayDateArrays();
}

uint16_t RGBDisplay::ColorPickerWheel(bool pick_new) {
  // if display is at min brightness then show constant sleep-friendly color, otherwise pick-new/old color
  if((current_brightness_ == (kNightBrightness + min_brightness_adder)) && sleep_friendly_color_at_night) {
    return kDisplayColorRed;
  }
  else {
    if(pick_new) {
      int newIndex = current_random_color_index_;
      while(newIndex == current_random_color_index_)
        newIndex = random(0, kColorPickerWheelSize - 1);
      current_random_color_index_ = newIndex;
      #ifdef MORE_LOGS
      PrintLn("ColorPickerWheel newIndex = ", newIndex);
      #endif
    }
    return kColorPickerWheelArray[current_random_color_index_];
  }
}

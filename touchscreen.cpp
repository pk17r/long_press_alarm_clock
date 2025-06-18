#include "touchscreen.h"
#include <SPI.h>
#include "rgb_display.h"
#include "nvs_preferences.h"

Touchscreen::Touchscreen() {
  touchscreen_type = nvs_preferences->RetrieveTouchscreenType();
  // PrintLn("touchscreen_type = ", touchscreen_type);

  int16_t xMin, xMax, yMin, yMax;
  nvs_preferences->RetrieveTouchScreenCalibration(xMin, xMax, yMin, yMax);

  if(touchscreen_type == 2) {       // MCU ADC
    touchscreen_r_ptr_ = new TouchscreenResistive(TOUCHSCREEN_XP, TOUCHSCREEN_XM_ADC(), TOUCHSCREEN_YP_ADC, TOUCHSCREEN_YM, 310);
    analogReadResolution(kAdcResolutionBits);
    touchscreen_r_ptr_->setAdcResolutionAndThreshold(kAdcResolutionBits);
    SetTouchscreenCalibration(xMin, xMax, yMin, yMax);
  }
  #ifdef XPT2046_OPTION
  else if(touchscreen_type == 1) {   // XPT2046
    touchscreen_ptr_ = new XPT2046_Touchscreen(TS_CS, TS_IRQ);
    touchscreen_ptr_->begin(*spi_obj);
    SetTouchscreenCalibration(xMin, xMax, yMin, yMax);
  }
  #endif

  SetTouchscreenOrientation();
  touchscreen_flip = nvs_preferences->RetrieveTouchscreenFlip();

  PrintLn(__func__, kInitializedStr);
}

Touchscreen::~Touchscreen() {
  if(touchscreen_r_ptr_ != NULL) {       // MCU ADC
    delete touchscreen_r_ptr_;
    // PrintLn("deleted touchscreen_r_ptr_");
  }
  #ifdef XPT2046_OPTION
  if(touchscreen_ptr_ != NULL) {        // XPT2046
    delete touchscreen_ptr_;
    // PrintLn("deleted touchscreen_ptr_");
  }
  #endif
}

void Touchscreen::SetTouchscreenOrientation() {
  if(touchscreen_type == 2) {       // MCU ADC
    if(display->screen_orientation_ == 1)
      touchscreen_r_ptr_->setRotation(3);
    else
      touchscreen_r_ptr_->setRotation(1);
  }
  #ifdef XPT2046_OPTION
  else if(touchscreen_type == 1) {   // XPT2046
    if(display->screen_orientation_ == 1)
      touchscreen_ptr_->setRotation(3);
    else
      touchscreen_ptr_->setRotation(1);
  }
  #endif
}

bool Touchscreen::IsTouched() {
  #ifdef XPT2046_OPTION
  if(touchscreen_type == 1) {   // XPT2046
    // irq touch is super fast
    if(!touchscreen_ptr_->tirqTouched())
      return false;
  }
  #endif

  if(inactivity_millis < kUserInputDelayMs) {
    // always read touch point if user is already engaged
    GetTouchedPixel();
    return last_touch_Pixel_.is_touched;
  }
  else if((inactivity_millis >= kUserInputDelayMs) && (millis() - last_polled_millis_ <= kPollingGapMs)) {
    // low power mode: if user is not engaged
    return last_touch_Pixel_.is_touched;
  }
  else {
    // read touch, if touched then read touch point
    if(touchscreen_type == 2) {       // MCU ADC
      last_touch_Pixel_.is_touched = touchscreen_r_ptr_->touched();
      if(last_touch_Pixel_.is_touched)
        GetTouchedPixel();
    }
    #ifdef XPT2046_OPTION
    else {    // if(touchscreen_type == 1)  // XPT2046
      // when irq touch is triggered, it takes a few hundred milliseconds to turn off
      // during this time we will poll touch screen pressure using SPI to know if touchscreen is pressed or not
      GetTouchedPixel();
    }
    #endif
    return last_touch_Pixel_.is_touched;
  }
}

TouchPixel* Touchscreen::GetTouchedPixel() {
  if(millis() - last_polled_millis_ <= kPollingGapMs) {
    // return last touch point
  }
  else {
    // note polling time
    last_polled_millis_ = millis();

    int16_t x = -1, y = -1;
    bool touched = GetUncalibratedTouch(x, y);

    if(!touched) {
      last_touch_Pixel_ = TouchPixel{-1, -1, false};
      return &last_touch_Pixel_;
    }

    // map
    x = max(min((int16_t)map(x, touchscreen_calibration_.xMin, touchscreen_calibration_.xMax, 0, touchscreen_calibration_.screen_width - 1), (int16_t)(touchscreen_calibration_.screen_width - 1)), (int16_t)0);
    y = max(min((int16_t)map(y, touchscreen_calibration_.yMin, touchscreen_calibration_.yMax, 0, touchscreen_calibration_.screen_height), (int16_t)(touchscreen_calibration_.screen_height - 1)), (int16_t)0);

    if(touchscreen_flip)
      y = touchscreen_calibration_.screen_height - y;

    last_touch_Pixel_ = TouchPixel{x, y, true};
  }
  return &last_touch_Pixel_;
}

bool Touchscreen::GetUncalibratedTouch(int16_t &x, int16_t &y) {
  int16_t z = -1;
  if(touchscreen_type == 2) {
    // get touch point using MCU ADC
    TsPoint touch = touchscreen_r_ptr_->getPoint();
    x = touch.x;
    y = touch.y;
    z = touch.z;
  }
  #ifdef XPT2046_OPTION
  else {    //if(touchscreen_type == 1)
    // get touch point from XPT2046
    TS_Point touch = touchscreen_ptr_->getPoint();
    x = touch.x;
    y = touch.y;
    z = touch.z;
  }
  #endif

  if(z < (touchscreen_type == 1 ? 100 : 1)) {
    return false;
  }

  // PrintLn(__func__, ("x:" + std::to_string(x) + ", y:" + std::to_string(y) + ", z:" + std::to_string(z)));
  return true;
}

void Touchscreen::SetTouchscreenCalibration(int16_t xMin, int16_t xMax, int16_t yMin, int16_t yMax) {
  touchscreen_calibration_ = TouchCalibration{xMin, xMax, yMin, yMax, kTftWidth, kTftHeight};  
}
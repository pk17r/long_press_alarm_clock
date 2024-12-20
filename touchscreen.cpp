#include "touchscreen.h"
#include <SPI.h>
#include "rgb_display.h"
#include "nvs_preferences.h"

Touchscreen::Touchscreen() {
  touchscreen_ptr_ = new XPT2046_Touchscreen(TS_CS_PIN, TS_IRQ_PIN);
  touchscreen_ptr_->begin(*spi_obj);
  SetTouchscreenOrientation();
  touchscreen_calibration_ = TouchCalibration{220, 3800, 280, 3830, kTftWidth, kTftHeight};
  touchscreen_flip = nvs_preferences->RetrieveTouchscreenFlip();

  PrintLn("Touchscreen Initialized!");
}

void Touchscreen::SetTouchscreenOrientation() {
  if(display->screen_orientation_ == 1)
    touchscreen_ptr_->setRotation(3);
  else
    touchscreen_ptr_->setRotation(1);
}

bool Touchscreen::IsTouched() {
  // irq touch is super fast
  if(!touchscreen_ptr_->tirqTouched())
    return false;
  // when irq touch is triggered, it takes a few hundred milliseconds to turn off
  // during this time we will poll touch screen pressure using SPI to know if touchscreen is pressed or not
  if(millis() - last_polled_millis_ <= kPollingGapMs) {
    return last_touch_Pixel_.is_touched;
  }
  else {
    GetTouchedPixel();
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

    // get touch point from XPT2046
    TS_Point touch = touchscreen_ptr_->getPoint();

    if(touch.z < 100) {
      last_touch_Pixel_ = TouchPixel{-1, -1, false};
      return &last_touch_Pixel_;
    }

    // Rotate and map
    int16_t x = map(touch.x, touchscreen_calibration_.xMin, touchscreen_calibration_.xMax, 0, touchscreen_calibration_.screen_width);
    int16_t y = 0;
    if(touchscreen_flip)
      y = touchscreen_calibration_.screen_height - map(touch.y, touchscreen_calibration_.yMin, touchscreen_calibration_.yMax, 0, touchscreen_calibration_.screen_height);
    else
      y = map(touch.y, touchscreen_calibration_.yMin, touchscreen_calibration_.yMax, 0, touchscreen_calibration_.screen_height);

    if (x > touchscreen_calibration_.screen_width){
        x = touchscreen_calibration_.screen_width;
    }
    if (x < 0) {
        x = 0;
    }
    if (y > touchscreen_calibration_.screen_height) {
        y = touchscreen_calibration_.screen_height;
    }
    if (y < 0) {
        y = 0;
    }
    last_touch_Pixel_ = TouchPixel{x, y, true};
  }
  return &last_touch_Pixel_;
}

#include "rgb_display.h"
#include "alarm_clock.h"
#include "wifi_stuff.h"
#include "rtc.h"
#include "touchscreen.h"
#include "nvs_preferences.h"

/*!
    @brief  Draw a 565 RGB image at the specified (x,y) position using monochrome 8-bit image.
            Converts each bitmap rows into 16-bit RGB buffer and sends over SPI.
            Adapted from Adafruit_SPITFT.cpp drawRGBBitmap function which is itself
            adapted from https://github.com/PaulStoffregen/ILI9341_t3
            by Marc MERLIN. See examples/pictureEmbed to use this.
            Handles its own transaction and edge clipping/rejection.
    @param  x        Top left corner horizontal coordinate.
    @param  y        Top left corner vertical coordinate.
    @param  bitmap   Pointer to 8-bit array of monochrome image
    @param  w        Width of bitmap in pixels.
    @param  h        Height of bitmap in pixels.
*/
void RGBDisplay::FastDrawTwoColorBitmapSpi(int16_t x, int16_t y, uint8_t *bitmap, int16_t w, int16_t h, uint16_t color, uint16_t bg) {
  int16_t x2, y2;                 // Lower-right coord
  if ((x >= kTftWidth) ||            // Off-edge right
      (y >= kTftHeight) ||           // " top
      ((x2 = (x + w - 1)) < 0) || // " left
      ((y2 = (y + h - 1)) < 0))
    return; // " bottom

  // elapsedMillis timer1;

  int bx1 = 0, by1 = 0, // Clipped top-left within bitmap
      saveW = w,            // Save original bitmap width value
      saveH = h;
  if (x < 0) {              // Clip left
    w += x;
    bx1 = -x;
    x = 0;
  }
  if (y < 0) { // Clip top
    h += y;
    by1 = -y;
    y = 0;
  }
  if (x2 >= kTftWidth)
    w = kTftWidth - x; // Clip right
  if (y2 >= kTftHeight)
    h = kTftHeight - y; // Clip bottom

  int16_t jLim = min(saveH, h + by1);
  int16_t iLim = min(saveW, w + bx1);

  // new 16 bit buffter of length w to hold 1 row colors
  uint16_t buffer16Bit[w];
  tft.startWrite();
  tft.setAddrWindow(x, y, w, h);

  int16_t bitmapWidthBytes = (saveW + 7) >> 3;          // bitmap width in bytes
  for (int16_t j = by1; j < jLim; j++) {
    int bufi = 0;
    int16_t i = bx1;
    uint8_t currentByte = bitmap[j * bitmapWidthBytes + (i >> 3)];
    uint8_t bitIndex = 7 - i % 8;
    while(1) {
      buffer16Bit[bufi] = (((currentByte >> bitIndex)  & 0x01) ? color : bg);
      bufi++;   // next row buffer index
      i++;  // next pixel
      if(i >= iLim)
        break;
      bitIndex = 7 - i % 8; // next bit index
      if(bitIndex == 7)  // new byte
        currentByte = bitmap[j * bitmapWidthBytes + (i >> 3)];
    }
    tft.writePixels(buffer16Bit, w);
  }
  tft.endWrite();
  // Serial.print(" fastDrawBitmapTime "); Serial.print(charSpace); Serial.println(timer1);
}

void RGBDisplay::SetAlarmScreen(bool processUserInput, bool inc_button_pressed, bool dec_button_pressed, bool push_button_pressed) {

  if(!processUserInput) {
    // make alarm set page

    tft.fillScreen(kDisplayBackroundColor);

    // set title font
    tft.setFont(&Satisfy_Regular24pt7b);

    char title[] = "Set Alarm";

    // change the text color to the background color
    tft.setTextColor(kDisplayBackroundColor);

    int16_t title_x0, title_y0;
    uint16_t title_w, title_h;
    // get bounds of title on tft display (with background color as this causes a blink)
    tft.getTextBounds(title, 0, 0, &title_x0, &title_y0, &title_w, &title_h);

    int16_t title_x = (kTftWidth - title_w) / 2;
    int16_t title_y = kTftHeight * 2 / 9;

    // change the text color to the Alarm color
    tft.setTextColor(kDisplayAlarmColor);
    tft.setCursor(title_x, title_y);
    tft.print(title);

    // font color inside
    tft.setTextColor(kDisplayColorGreen);

    // draw alarm page buttons
    AlarmPageButtonFn(BTN_HOUR, BTN_DRAW, kBtnHighlightFlag);
    AlarmPageButtonFn(BTN_MINU, BTN_DRAW, 0);
    AlarmPageButtonFn(BTN_AMPM, BTN_DRAW, 0);
    AlarmPageButtonFn(BTN_ONOF, BTN_DRAW, 0);
    AlarmPageButtonFn(BTN_SETX, BTN_DRAW, 0);
  }
  else {
    // processUserInput

    // userButtonClick
    //    0 = no button clicked or Push Button Pressed (Move to next Operation)
    //    1 = Hr Dec button
    //    2 = Hr Inc button
    //    3 = Min Dec button
    //    4 = Min Inc button
    //    5 = AmPm Dec button
    //    6 = AmPm Inc button
    //    7 = Alarm On button
    //    8 = Alarm Off button
    //    9 = Set button
    //    10 = Cancel button
    int16_t userButtonClick = 0;

    // if push_button_pressed or touchscreen input then un-highlight current cursor
    if(push_button_pressed || (!inc_button_pressed && !dec_button_pressed && !push_button_pressed)) {
      // un-highlight current cursor
      switch (current_cursor)
      {
        case kAlarmSetPageHour:
          AlarmPageButtonFn(BTN_HOUR, BTN_HIGHLIGHT, 0);
          break;
        case kAlarmSetPageMinute:
          AlarmPageButtonFn(BTN_MINU, BTN_HIGHLIGHT, 0);
          break;
        case kAlarmSetPageAmPm:
          AlarmPageButtonFn(BTN_AMPM, BTN_HIGHLIGHT, 0);
          break;
        case kAlarmSetPageOn:
        case kAlarmSetPageOff:
          AlarmPageButtonFn(BTN_ONOF, BTN_HIGHLIGHT, 0);
          break;
        case kAlarmSetPageSet:
        case kAlarmSetPageCancel:
          AlarmPageButtonFn(BTN_SETX, BTN_HIGHLIGHT, 0);
          break;
        default:
          break;
      }
    }

    // note user input - via touchscreen or pushbuttons
    if(!inc_button_pressed && !dec_button_pressed && !push_button_pressed) {
      // touch screen input

      // find if user clicked a button
      if(AlarmPageButtonFn(BTN_HOUR, BTN_ISTOUCHED_TOP, 0)){
        userButtonClick = 1;
        current_cursor = kAlarmSetPageHour;
      }
      else if(AlarmPageButtonFn(BTN_HOUR, BTN_ISTOUCHED_BTM, 0)){
        userButtonClick = 2;
        current_cursor = kAlarmSetPageHour;
      }
      else if(AlarmPageButtonFn(BTN_MINU, BTN_ISTOUCHED_TOP, 0)){
        userButtonClick = 3;
        current_cursor = kAlarmSetPageMinute;
      }
      else if(AlarmPageButtonFn(BTN_MINU, BTN_ISTOUCHED_BTM, 0)){
        userButtonClick = 4;
        current_cursor = kAlarmSetPageMinute;
      }
      else if(AlarmPageButtonFn(BTN_AMPM, BTN_ISTOUCHED_TOP, 0)){
        userButtonClick = 5;
        current_cursor = kAlarmSetPageAmPm;
      }
      else if(AlarmPageButtonFn(BTN_AMPM, BTN_ISTOUCHED_BTM, 0)){
        userButtonClick = 6;
        current_cursor = kAlarmSetPageAmPm;
      }
      else if(AlarmPageButtonFn(BTN_ONOF, BTN_ISTOUCHED_TOP, 0)){
        userButtonClick = 7;
        current_cursor = kAlarmSetPageOn;
      }
      else if(AlarmPageButtonFn(BTN_ONOF, BTN_ISTOUCHED_BTM, 0)){
        userButtonClick = 8;
        current_cursor = kAlarmSetPageOff;
      }
      else if(AlarmPageButtonFn(BTN_SETX, BTN_ISTOUCHED_TOP, 0)){
        userButtonClick = 9;
        current_cursor = kAlarmSetPageSet;
      }
      else if(AlarmPageButtonFn(BTN_SETX, BTN_ISTOUCHED_BTM, 0)){
        userButtonClick = 10;
        current_cursor = kAlarmSetPageCancel;
      }
    }
    else {    // button input       //inc_button_pressed || dec_button_pressed || push_button_pressed

      // userButtonClick
      //    0 = no button clicked or Push Button Pressed (Move to next Operation)
      //    1 = Hr Dec button
      //    2 = Hr Inc button
      //    3 = Min Dec button
      //    4 = Min Inc button
      //    5 = AmPm Dec button
      //    6 = AmPm Inc button
      //    7 = Alarm On button
      //    8 = Alarm Off button
      //    9 = Set button
      //    10 = Cancel button

      // if no cursor selection then reset cursor to Hour button
      if(current_cursor == kCursorNoSelection)
        current_cursor = kAlarmSetPageHour;

      // set new cursor position
      if(current_cursor == kAlarmSetPageHour) {
        if(inc_button_pressed) userButtonClick = 2;
        else if(dec_button_pressed) userButtonClick = 1;
        else {// push_button_pressed
          current_cursor = kAlarmSetPageMinute;
          // highlight next button
          AlarmPageButtonFn(BTN_MINU, BTN_HIGHLIGHT, kBtnHighlightFlag);
        }
      }
      else if(current_cursor == kAlarmSetPageMinute) {
        if(inc_button_pressed) userButtonClick = 4;
        else if(dec_button_pressed) userButtonClick = 3;
        else {// push_button_pressed
          current_cursor = kAlarmSetPageAmPm;
          // highlight next button
          AlarmPageButtonFn(BTN_AMPM, BTN_HIGHLIGHT, kBtnHighlightFlag);
        }
      }
      else if(current_cursor == kAlarmSetPageAmPm) {
        if(inc_button_pressed) userButtonClick = 6;
        else if(dec_button_pressed) userButtonClick = 5;
        else {// push_button_pressed
          current_cursor = kAlarmSetPageOn;
          // highlight next button
          AlarmPageButtonFn(BTN_ONOF, BTN_HIGHLIGHT, kBtnHighlightTopFlag);
        }
      }
      else if(current_cursor == kAlarmSetPageOn || current_cursor == kAlarmSetPageOff) {
        if(inc_button_pressed || dec_button_pressed) {
          if(current_cursor == kAlarmSetPageOn)
            current_cursor = kAlarmSetPageOff;
          else
            current_cursor = kAlarmSetPageOn;
          // highlight button
          AlarmPageButtonFn(BTN_ONOF, BTN_HIGHLIGHT, (current_cursor == kAlarmSetPageOn ? kBtnHighlightTopFlag : kBtnHighlightBtmFlag));
        }
        else {// push_button_pressed
          if(current_cursor == kAlarmSetPageOn)
            userButtonClick = 7;
          else
            userButtonClick = 8;
        }
      }
      else if(current_cursor == kAlarmSetPageSet || current_cursor == kAlarmSetPageCancel) {
        if(inc_button_pressed || dec_button_pressed) {
          if(current_cursor == kAlarmSetPageSet)
            current_cursor = kAlarmSetPageCancel;
          else
            current_cursor = kAlarmSetPageSet;
          // highlight other button
          AlarmPageButtonFn(BTN_SETX, BTN_HIGHLIGHT, (kBtnHighlightFlag | (current_cursor == kAlarmSetPageSet ? kBtnHighlightTopFlag : kBtnHighlightBtmFlag)));
        }
        else {// push_button_pressed
          if(current_cursor == kAlarmSetPageSet)
            userButtonClick = 9;
          else
            userButtonClick = 10;
        }
      }
    }

    // Process user input
    // userButtonClick
    //    0 = no button clicked or Push Button Pressed (Move to next Operation)
    //    1 = Hr Dec button
    //    2 = Hr Inc button
    //    3 = Min Dec button
    //    4 = Min Inc button
    //    5 = AmPm Dec button
    //    6 = AmPm Inc button
    //    7 = Alarm On button
    //    8 = Alarm Off button
    //    9 = Set button
    //    10 = Cancel button

    // highlight userButtonClick and update values
    if(userButtonClick == 1 || userButtonClick == 2) {        // Hours Roller
      current_cursor = kAlarmSetPageHour;
      uint8_t currentBtnFlags = kBtnHighlightFlag;
      if(userButtonClick == 1) {  // Hr Dec button
        alarm_clock->var_1_--;
        currentBtnFlags |= kBtnHighlightTopFlag;
      }
      else {                      // Hr Inc button
        alarm_clock->var_1_++;
        currentBtnFlags |= kBtnHighlightBtmFlag;
      }
      // check overflow
      if(alarm_clock->var_1_ == 0)
        alarm_clock->var_1_ = 12;
      if(alarm_clock->var_1_ == 13)
        alarm_clock->var_1_ = 1;
      // fill triangle button
      AlarmPageButtonFn(BTN_HOUR, BTN_DRAW, currentBtnFlags);
      // wait a little
      delay(kUserInputDelayMs);
      // unfill triangle button
      AlarmPageButtonFn(BTN_HOUR, BTN_HIGHLIGHT, kBtnHighlightFlag);
    }
    else if(userButtonClick == 3 || userButtonClick == 4) {     // Minutes Roller
      current_cursor = kAlarmSetPageMinute;
      uint8_t currentBtnFlags = kBtnHighlightFlag;
        if(userButtonClick == 3) {  // Min Dec button
          alarm_clock->var_2_--;
          // check underflow
          if(alarm_clock->var_2_ > 100)
            alarm_clock->var_2_ = 59;
          currentBtnFlags |= kBtnHighlightTopFlag;
        }
        else {                      // Min Inc button
          alarm_clock->var_2_++;
          // check overflow
          if(alarm_clock->var_2_ >= 60)
            alarm_clock->var_2_ -= 60;
          currentBtnFlags |= kBtnHighlightBtmFlag;
        }
        // fill triangle button
        AlarmPageButtonFn(BTN_MINU, BTN_DRAW, currentBtnFlags);
        // NO wait for Minutes Roller Button
        // delay(kUserInputDelayMs);
        // unfill triangle button
        AlarmPageButtonFn(BTN_MINU, BTN_HIGHLIGHT, kBtnHighlightFlag);
    }
    else if(userButtonClick == 5 || userButtonClick == 6) {       // AM PM Roller
      current_cursor = kAlarmSetPageAmPm;
      uint8_t currentBtnFlags = kBtnHighlightFlag;
      alarm_clock->var_3_is_AM_ = !alarm_clock->var_3_is_AM_;
      if(userButtonClick == 5)
        currentBtnFlags |= kBtnHighlightTopFlag;
      else
        currentBtnFlags |= kBtnHighlightBtmFlag;
      // fill triangle button
      AlarmPageButtonFn(BTN_AMPM, BTN_DRAW, currentBtnFlags);
      // wait a little
      delay(kUserInputDelayMs);
      // unfill triangle button
      AlarmPageButtonFn(BTN_AMPM, BTN_HIGHLIGHT, kBtnHighlightFlag);
    }
    else if(userButtonClick == 7 || userButtonClick == 8) {       // On / Off Button
      uint8_t currentBtnFlags = 0;
      if(userButtonClick == 7) {
        current_cursor = kAlarmSetPageOn;
        alarm_clock->var_4_ON_ = true;
        currentBtnFlags |= kBtnHighlightTopFlag;
      }
      else {
        current_cursor = kAlarmSetPageOff;
        alarm_clock->var_4_ON_ = false;
        currentBtnFlags |= kBtnHighlightBtmFlag;
      }
      // draw button with press indication
      AlarmPageButtonFn(BTN_ONOF, BTN_DRAW, (currentBtnFlags | kBtnPressFlag));
      // wait a little
      delay(kUserInputDelayMs);
      // draw button without press indication
      AlarmPageButtonFn(BTN_ONOF, BTN_DRAW, currentBtnFlags);

      // if push button was pressed then move cursor
      if(push_button_pressed) {
        // unhighlight current button
        AlarmPageButtonFn(BTN_ONOF, BTN_HIGHLIGHT, 0);
        // move cursor
        current_cursor = kAlarmSetPageSet;
        AlarmPageButtonFn(BTN_SETX, BTN_HIGHLIGHT, kBtnHighlightTopFlag);
      }
    }
    else if(userButtonClick == 9 || userButtonClick == 10) {      // Set / Cancel Button
      uint8_t currentBtnFlags = kBtnPressFlag;
      if(userButtonClick == 9) {    // set button pressed
        current_cursor = kAlarmSetPageSet;
        currentBtnFlags |= kBtnHighlightTopFlag;
        // save Set Alarm Page values
        alarm_clock->SaveAlarm();
      }
      else {
        current_cursor = kAlarmSetPageCancel;
        currentBtnFlags |= kBtnHighlightBtmFlag;
      }
      // draw button with highlight
      AlarmPageButtonFn(BTN_SETX, BTN_DRAW, currentBtnFlags);
      // wait a little
      delay(2 * kUserInputDelayMs);
      // go back to main page
      SetPage(kMainPage);
    }
  }
}

bool RGBDisplay::AlarmPageButtonFn(AlarmPageBtnType btn_type, AlarmPageBtnAction btn_action, uint8_t btn_highlight_flags) {
  const int16_t btnMidY = kTftHeight * 2 / 3;
  int16_t btnMidX = kTftWidth / 10;
  if(btn_type == BTN_HOUR) btnMidX *= 1;
  else if(btn_type == BTN_MINU) btnMidX *= 3;
  else if(btn_type == BTN_AMPM) btnMidX *= 5;
  else if(btn_type == BTN_ONOF) btnMidX *= 7;
  else if(btn_type == BTN_SETX) btnMidX *= 9;
  
  const uint16_t kBtnBoundaryW = 64, kBtnBoundaryH = 160, kGap = 5;
  // roller button constants
  const uint16_t kTriangleBtnH = kBtnBoundaryH / 4, kLabelH = kBtnBoundaryH / 2;
  const int16_t kTriangleX1 = btnMidX - kBtnBoundaryW / 2 + kGap, kTriangleX2 = btnMidX + kBtnBoundaryW / 2 - kGap;
  const int16_t kTriangleY1Top = btnMidY - kBtnBoundaryH / 2, kTriangleY2Top = kTriangleY1Top + kTriangleBtnH;
  const int16_t kTriangleY2Btm = btnMidY + kBtnBoundaryH / 2, kTriangleY1Btm = kTriangleY2Btm - kTriangleBtnH;
  // on off button constants
  const uint16_t kBtnW = kBtnBoundaryW - 2 * kGap, kBtnH = kBtnBoundaryH / 2 - 2 * kGap, kBtnHighlightH = kBtnBoundaryH / 2;
  const int16_t kBtnX = btnMidX - kBtnW / 2, kBtnHighlightX = btnMidX - kBtnBoundaryW / 2;
  const int16_t kBtn1Y = btnMidY - kBtnH - kGap, kBtn2Y = btnMidY + kGap;
  const int16_t kBtn1HighlightY = btnMidY - kBtnBoundaryH / 2, kBtn2HighlightY = btnMidY;

  // button colors
  const uint16_t onFill = kDisplayColorGreen, offFill = kDisplayBackroundColor, borderColor = kDisplayColorCyan;

  switch (btn_action)
  {
    case BTN_DRAW:
      if(btn_type <= BTN_AMPM)          // Roller Button
      {
        // draw label
        {
          char label_arr[3];
          if(btn_type <= BTN_MINU) {
            uint8_t label1_num = (btn_type == BTN_HOUR ? alarm_clock->var_1_ : alarm_clock->var_2_);
            if(label1_num / 10 > 0)
              label_arr[0] = label1_num / 10 + 48;    // first digit = number
            else if(btn_type == BTN_HOUR)
              label_arr[0] = 32;                   // first digit = space
            else  // btn_type == BTN_MINU
              label_arr[0] = 48;                   // first digit = zero
            label_arr[1] = label1_num % 10 + 48;    // second digit
            label_arr[2] = '\0';
          }
          else {
            label_arr[0] = (alarm_clock->var_3_is_AM_ ? kAmLabel[0] : kPmLabel[0]);
            label_arr[1] = 'M';
            label_arr[2] = '\0';
          }
          // write label
          int16_t label_x0, label_y0;
          uint16_t label_w, label_h;
          // get bounds of title on tft display (with background color as this causes a blink)
          tft.setTextColor(offFill);
          if(btn_type == BTN_AMPM)
            tft.setFont(&FreeSans18pt7b);
          else
            tft.setFont(&Satisfy_Regular24pt7b);
          tft.getTextBounds(label_arr, 0, 0, &label_x0, &label_y0, &label_w, &label_h);
          // std::string debug_text = std::string(label_arr) + " : label_x0=" + std::to_string(label_x0) + ", label_y0=" + std::to_string(label_y0) + ", label_w=" + std::to_string(label_w) + ", label_h=" + std::to_string(label_h);
          // PrintLn(debug_text);
          // clear old label
          tft.fillRoundRect(btnMidX - kBtnBoundaryW / 2 + 1, btnMidY - kLabelH / 2 + 1, kBtnBoundaryW - 2, kLabelH - 2, kRadiusButtonRoundRect, kDisplayBackroundColor);
          tft.setTextColor(kDisplayColorGreen);
          label_x0 = btnMidX - label_w / 2 - /* getTextBounds() Start Position */ label_x0;
          label_y0 = btnMidY + label_h / 2;
          tft.setCursor(label_x0, label_y0);
          tft.print(label_arr);
        }
        // draw roller button label highlight
        tft.drawRoundRect(btnMidX - kBtnBoundaryW / 2 + kGap, btnMidY - kLabelH / 2 + kGap, kBtnBoundaryW - 2 * kGap, kLabelH - 2 * kGap, kRadiusButtonRoundRect, ((btn_highlight_flags & kBtnHighlightFlag) ? borderColor : kDisplayBackroundColor));
        // top and btm triangle buttons with highlight fills
        tft.fillTriangle(kTriangleX1, kTriangleY2Top, btnMidX, kTriangleY1Top, kTriangleX2, kTriangleY2Top, ((btn_highlight_flags & kBtnHighlightTopFlag) ? kButtonClickedFillColor : offFill));
        tft.drawTriangle(kTriangleX1, kTriangleY2Top, btnMidX, kTriangleY1Top, kTriangleX2, kTriangleY2Top, borderColor);
        tft.fillTriangle(kTriangleX1, kTriangleY1Btm, btnMidX, kTriangleY2Btm, kTriangleX2, kTriangleY1Btm, ((btn_highlight_flags & kBtnHighlightBtmFlag) ? kButtonClickedFillColor : offFill));
        tft.drawTriangle(kTriangleX1, kTriangleY1Btm, btnMidX, kTriangleY2Btm, kTriangleX2, kTriangleY1Btm, borderColor);
      }
      else {                            // On Off Button
        // undraw button highlights
        tft.drawRoundRect(kBtnHighlightX, kBtn1HighlightY, kBtnBoundaryW, kBtnHighlightH, kRadiusButtonRoundRect, kDisplayBackroundColor);
        tft.drawRoundRect(kBtnHighlightX, kBtn2HighlightY, kBtnBoundaryW, kBtnHighlightH, kRadiusButtonRoundRect, kDisplayBackroundColor);
        // draw button 1
        bool isBtnPressed = ((btn_highlight_flags & kBtnPressFlag) && (btn_highlight_flags & kBtnHighlightTopFlag));
        if(btn_type == BTN_ONOF)
          DrawButton(kBtnX, kBtn1Y, kBtnW, kBtnH, "ON", borderColor, (isBtnPressed ? kButtonClickedFillColor : onFill), offFill, (isBtnPressed | alarm_clock->var_4_ON_));
        else
          DrawButton(kBtnX, kBtn1Y, kBtnW, kBtnH, "SET", borderColor, (isBtnPressed ? kButtonClickedFillColor : kButtonFillColor), offFill, true);
        // draw button 2
        isBtnPressed = ((btn_highlight_flags & kBtnPressFlag) && (btn_highlight_flags & kBtnHighlightBtmFlag));
        if(btn_type == BTN_ONOF)
          DrawButton(kBtnX, kBtn2Y, kBtnW, kBtnH, "OFF", borderColor, (isBtnPressed ? kButtonClickedFillColor : onFill), offFill, (isBtnPressed | !(alarm_clock->var_4_ON_)));
        else
          DrawButton(kBtnX, kBtn2Y, kBtnW, kBtnH, kCancelStr, borderColor, (isBtnPressed ? kButtonClickedFillColor : kButtonFillColor), offFill, true);
        // draw button highlights
        tft.drawRoundRect(kBtnHighlightX, kBtn1HighlightY, kBtnBoundaryW, kBtnHighlightH, kRadiusButtonRoundRect, ((btn_highlight_flags & kBtnHighlightTopFlag) ? borderColor : kDisplayBackroundColor));
        tft.drawRoundRect(kBtnHighlightX, kBtn2HighlightY, kBtnBoundaryW, kBtnHighlightH, kRadiusButtonRoundRect, ((btn_highlight_flags & kBtnHighlightBtmFlag) ? borderColor : kDisplayBackroundColor));
      }
      break;
    case BTN_HIGHLIGHT:
      if(btn_type <= BTN_AMPM) {        // Roller Button
        // draw roller button highlight
        tft.drawRoundRect(btnMidX - kBtnBoundaryW / 2 + kGap, btnMidY - kLabelH / 2 + kGap, kBtnBoundaryW - 2 * kGap, kLabelH - 2 * kGap, kRadiusButtonRoundRect, ((btn_highlight_flags & kBtnHighlightFlag) ? borderColor : kDisplayBackroundColor));
        // top and btm triangle buttons with highlight fills
        tft.fillTriangle(kTriangleX1, kTriangleY2Top, btnMidX, kTriangleY1Top, kTriangleX2, kTriangleY2Top, ((btn_highlight_flags & kBtnHighlightTopFlag) ? kButtonClickedFillColor : offFill));
        tft.drawTriangle(kTriangleX1, kTriangleY2Top, btnMidX, kTriangleY1Top, kTriangleX2, kTriangleY2Top, borderColor);
        tft.fillTriangle(kTriangleX1, kTriangleY1Btm, btnMidX, kTriangleY2Btm, kTriangleX2, kTriangleY1Btm, ((btn_highlight_flags & kBtnHighlightBtmFlag) ? kButtonClickedFillColor : offFill));
        tft.drawTriangle(kTriangleX1, kTriangleY1Btm, btnMidX, kTriangleY2Btm, kTriangleX2, kTriangleY1Btm, borderColor);
      }
      else {                            // On Off Button
        // draw button highlights
        tft.drawRoundRect(kBtnHighlightX, kBtn1HighlightY, kBtnBoundaryW, kBtnHighlightH, kRadiusButtonRoundRect, ((btn_highlight_flags & kBtnHighlightTopFlag) ? borderColor : kDisplayBackroundColor));
        tft.drawRoundRect(kBtnHighlightX, kBtn2HighlightY, kBtnBoundaryW, kBtnHighlightH, kRadiusButtonRoundRect, ((btn_highlight_flags & kBtnHighlightBtmFlag) ? borderColor : kDisplayBackroundColor));
      }
      break;
    case BTN_ISTOUCHED_TOP:
      if(ts != nullptr) {
        // touch screen input
        int16_t ts_x = ts->GetTouchedPixel()->x, ts_y = ts->GetTouchedPixel()->y;
        if(btn_type <= BTN_AMPM) {        // Roller Button
          if(ts_x >= kTriangleX1 && ts_x <= kTriangleX2 && ts_y >= kTriangleY1Top - kGap && ts_y <= kTriangleY2Top + kGap)
            return true;
        }
        else {                            // On Off Button
          if(ts_x >= kBtnX && ts_x <= kBtnX + kBtnW && ts_y >= kBtn1Y && ts_y <= kBtn1Y + kBtnH)
            return true;
        }
      }
      break;
    case BTN_ISTOUCHED_BTM:
      if(ts != nullptr) {
        // touch screen input
        int16_t ts_x = ts->GetTouchedPixel()->x, ts_y = ts->GetTouchedPixel()->y;
        if(btn_type <= BTN_AMPM) {        // Roller Button
          if(ts_x >= kTriangleX1 && ts_x <= kTriangleX2 && ts_y >= kTriangleY1Btm - kGap && ts_y <= kTriangleY2Btm + kGap)
            return true;
        }
        else {                            // On Off Button
          if(ts_x >= kBtnX && ts_x <= kBtnX + kBtnW && ts_y >= kBtn2Y && ts_y <= kBtn2Y + kBtnH)
            return true;
        }
      }
      break;
    default:
      break;
  }
  return false;
}

void RGBDisplay::DrawButton(int16_t x, int16_t y, uint16_t w, uint16_t h, const char* label, uint16_t borderColor, uint16_t onFill, uint16_t offFill, bool isOn) {
  if(current_page == kAlarmSetPage)
    tft.setFont(&FreeSansBold12pt7b);
  else
    tft.setFont(&FreeMonoBold9pt7b);
  tft.setTextColor((isOn ? onFill : offFill));
  int16_t label_x0, label_y0;
  uint16_t label_w, label_h;
  // get bounds of title on tft display (with background color as this causes a blink)
  tft.getTextBounds(label, x, y + h, &label_x0, &label_y0, &label_w, &label_h);
  // make button
  tft.fillRoundRect(x, y, w, h, kRadiusButtonRoundRect, (isOn ? onFill : offFill));
  tft.drawRoundRect(x, y, w, h, kRadiusButtonRoundRect, borderColor);
  tft.setTextColor((isOn ? offFill : onFill));
  label_x0 = x + (w - label_w) / 2 -  /* getTextBounds() Start Position */ (label_x0 - x);
  label_y0 = y + h / 2 + label_h / 2;
  tft.setCursor(label_x0, label_y0);
  tft.print(label);
}

void RGBDisplay::DrawTriangleButton(int16_t x, int16_t y, uint16_t w, uint16_t h, bool isUp, uint16_t borderColor, uint16_t fillColor) {
  int16_t x1, y1, x2, y2, x3, y3;
  if(isUp) {
    x1 = x + w / 2;
    y1 = y;
    x2 = x;
    y2 = y + h;
    x3 = x + w;
    y3 = y + h;
  }
  else {
    x1 = x;
    y1 = y;
    x2 = x + w / 2;
    y2 = y + h;
    x3 = x + w;
    y3 = y;
  }
  // make button
  tft.fillTriangle(x1, y1, x2, y2, x3, y3, fillColor);
  tft.drawTriangle(x1, y1, x2, y2, x3, y3, borderColor);
}

void RGBDisplay::DisplayCursorHighlight(DisplayButton* button, bool highlight_On) {
  // special case first
  if((current_page == kWiFiScanNetworksPage) && (current_cursor == kWiFiScanNetworksPageList)) {
    // inside WiFi Scan Networks Page List
    int cursorY = kWiFiScanNetworksList_y0_ - 0.75 * kWiFiScanNetworksList_h_ + 1 + (kWiFiScanNetworksList_h_ * current_wifi_networks_scan_page_cursor);
    if(highlight_On) {
      tft.drawRoundRect(0, cursorY, kTftWidth - 1, kWiFiScanNetworksList_h_, kRadiusButtonRoundRect, kDisplayColorCyan);
    }
    else {
      tft.drawRoundRect(0, cursorY, kTftWidth - 1, kWiFiScanNetworksList_h_, kRadiusButtonRoundRect, kDisplayBackroundColor);
    }
  }
  else {
    if(highlight_On) {
      tft.drawRoundRect(button->btn_x - 1, button->btn_y - 1, button->btn_w + 2 * 1, button->btn_h + 2 * 1, kRadiusButtonRoundRect, kDisplayColorCyan);
      tft.drawRoundRect(button->btn_x - 2, button->btn_y - 2, button->btn_w + 2 * 2, button->btn_h + 2 * 2, kRadiusButtonRoundRect, kDisplayColorCyan);
    }
    else {
      tft.drawRoundRect(button->btn_x - 1, button->btn_y - 1, button->btn_w + 2 * 1, button->btn_h + 2 * 1, kRadiusButtonRoundRect, kDisplayBackroundColor);
      tft.drawRoundRect(button->btn_x - 2, button->btn_y - 2, button->btn_w + 2 * 2, button->btn_h + 2 * 2, kRadiusButtonRoundRect, kDisplayBackroundColor);
    }
  }
}

void RGBDisplay::DisplayCursorHighlight(bool highlight_On) {
  for (int i = 0; i < display_pages_vec[current_page].size(); i++) {
    DisplayButton* button = display_pages_vec[current_page][i];
    if(button->btn_cursor_id == current_cursor) {
      DisplayCursorHighlight(button, highlight_On);
      break;
    }
  }
}

void RGBDisplay::DisplayCurrentPageButtonRow(DisplayButton* button, int button_index, bool is_on) {

  // exclude special case: (current_page == kWiFiScanNetworksPage) && (current_cursor == kWiFiScanNetworksPageList) -> these list items are just label
  if((current_page != kWiFiScanNetworksPage) || (current_cursor != kWiFiScanNetworksPageList)) {

    const int row_text_y0 = (button_index + 1) * kPageRowHeight + 20;

    // clear row if item has label
    if(button->row_label.size() > 0)
      tft.fillRect(0, row_text_y0 - 20, kTftWidth, kPageRowHeight, kDisplayBackroundColor);

    int space_left = kTftWidth;

    // First Row Item's Button is drawn, then Row Item's Label is drawn

    // item button

    // if not an icon button then make a button
    if(button->btn_type != kClickButtonWithIcon) {
      if(button->fixed_location) {
        // pre-fixed location button
        tft.setFont(&FreeMonoBold9pt7b);
        tft.setTextColor(kDisplayBackroundColor);
        // make button
        tft.fillRoundRect(button->btn_x, button->btn_y, button->btn_w, button->btn_h, kRadiusButtonRoundRect, (is_on ? kButtonClickedFillColor : kButtonFillColor));
        tft.drawRoundRect(button->btn_x, button->btn_y, button->btn_w, button->btn_h, kRadiusButtonRoundRect, kButtonBorderColor);
        int16_t btn_value_x0 = button->btn_x + kDisplayTextGap, btn_value_y0 = button->btn_y + button->btn_h - kDisplayTextGap;
        tft.setCursor(btn_value_x0, btn_value_y0);
        tft.print(button->btn_value.c_str());
      }
      else {
        // button location to be calculated at runtime
        tft.setFont(&FreeMonoBold9pt7b);
        tft.setTextColor(kDisplayBackroundColor);
        int16_t btn_value_x0 = 0, btn_value_y0 = row_text_y0;
        uint16_t btn_value_w, btn_value_h;
        tft.setCursor(btn_value_x0, row_text_y0);
        // get bounds of title on tft display (with background color as this causes a blink)
        tft.getTextBounds(button->btn_value.c_str(), btn_value_x0, btn_value_y0, &btn_value_x0, &btn_value_y0, &btn_value_w, &btn_value_h);
        // Serial.printf("btn_value_x0 %d, btn_value_y0 %d, btn_value_w %d, btn_value_h %d\n", btn_value_x0, btn_value_y0, btn_value_w, btn_value_h);
        // calculate size
        button->btn_x = kTftWidth - btn_value_w - 3 * kDisplayTextGap;
        button->btn_y = row_text_y0 - btn_value_h - kDisplayTextGap;
        button->btn_w = btn_value_w + 2 * kDisplayTextGap;
        button->btn_h = btn_value_h + 2 * kDisplayTextGap;
        // Serial.printf("flexible button->btn_x %d, button->btn_y %d, button->btn_w %d, button->btn_h %d\n", button->btn_x, button->btn_y, button->btn_w, button->btn_h);
        // make button
        if(button->btn_type == kLabelOnlyNoClickButton) {
          // special case -> only label, no click button
          tft.setTextColor(kDisplayColorGreen);
          button->btn_x = button->btn_x + 2 * kDisplayTextGap;
          tft.setCursor(button->btn_x, row_text_y0);
          tft.print(button->btn_value.c_str());
        }
        else {
          tft.fillRoundRect(button->btn_x, button->btn_y, button->btn_w, button->btn_h, kRadiusButtonRoundRect, (is_on ? kButtonClickedFillColor : kButtonFillColor));
          tft.drawRoundRect(button->btn_x, button->btn_y, button->btn_w, button->btn_h, kRadiusButtonRoundRect, kButtonBorderColor);
          tft.setCursor(button->btn_x + kDisplayTextGap, row_text_y0);
          tft.print(button->btn_value.c_str());
        }
      }

      space_left = button->btn_x - kDisplayTextGap;
    }

    // item label

    if(button->row_label.size() > 0) {
      tft.setFont(&FreeMono9pt7b);
      tft.setTextColor(kDisplayBackroundColor);
      int16_t row_label_x0 = 0, row_label_y0 = row_text_y0;
      uint16_t row_label_w, row_label_h;
      tft.setCursor(row_label_x0, row_text_y0);
      // get bounds of title on tft display (with background color as this causes a blink)
      tft.getTextBounds(button->row_label.c_str(), row_label_x0, row_label_y0, &row_label_x0, &row_label_y0, &row_label_w, &row_label_h);
      // Serial.printf("row_label_x0 %d, row_label_y0 %d, row_label_w %d, row_label_h %d\n", row_label_x0, row_label_y0, row_label_w, row_label_h);
      // check width and fit in 1 or 2 rows
      if(row_label_w + kDisplayTextGap <= space_left) {
        // label fits in 1 row
        tft.setTextColor(kDisplayColorYellow);
        tft.setCursor(kDisplayTextGap, row_text_y0);
        tft.print(button->row_label.c_str());
      }
      else {
        // we give label 2 rows
        tft.setTextColor(kDisplayColorYellow);
        // row 1
        tft.setCursor(kDisplayTextGap, row_text_y0 - 10);
        int row_1_label_length = button->row_label.size() / 2;
        std::string row_1_label = button->row_label.substr(0, row_1_label_length);
        tft.print(row_1_label.c_str());
        tft.setCursor(kDisplayTextGap, row_text_y0 + 5);
        std::string row_2_label = button->row_label.substr(row_1_label_length, button->row_label.size() - row_1_label_length);
        tft.print(row_2_label.c_str());
      }

      // Special case -> Clock settings page city
      if(current_page == kClockSettingsPage && button->btn_cursor_id == kClockSettingsPageSetLocation && wifi_stuff->city_.length() > 0) {
        tft.setFont(&FreeMonoBold9pt7b);
        tft.setTextColor(kDisplayColorGreen);
        tft.print(wifi_stuff->city_.c_str());
      }
    }
  }

  // button highlight
  if(button->btn_cursor_id == current_cursor)
    DisplayCursorHighlight(button, true);
  else
    DisplayCursorHighlight(button, false);
}

void RGBDisplay::DisplayCurrentPageButtonRow(int button_index, bool is_on) {
  DisplayButton* button = display_pages_vec[current_page][button_index];
  DisplayCurrentPageButtonRow(button, button_index, is_on);
}

void RGBDisplay::DisplayCurrentPageButtonRow(bool is_on) {
  for (int i = 0; i < display_pages_vec[current_page].size(); i++) {
    if(display_pages_vec[current_page][i]->btn_cursor_id == current_cursor) {
      DisplayButton* button = display_pages_vec[current_page][i];
      DisplayCurrentPageButtonRow(button, i, is_on);
      break;
    }
  }
}

void RGBDisplay::DisplayCurrentPage() {
  tft.fillScreen(kDisplayBackroundColor);

  // Page Title
  tft.setFont(&FreeMonoBold9pt7b);
  tft.setTextColor(kDisplayColorGreen);
  tft.setCursor(kDisplayTextGap, 20);
  std::string title_str = "";
  switch(current_page) {
    case kScreensaverSettingsPage: title_str = "SCREENSAVER SETTINGS PAGE"; break;
    case kSettingsPage: title_str = "MAIN SETTINGS PAGE"; break;
    case kWiFiSettingsPage: title_str = "WIFI SETTINGS PAGE"; break;
    case kClockSettingsPage: title_str = "CLOCK SETTINGS PAGE"; break;
    case kWeatherSettingsPage: title_str = "WEATHER PAGE"; break;
    default: title_str = "Not Implemented!";
  }
  tft.print(title_str.c_str());

  // Page Body
  for (int i = 0; i < display_pages_vec[current_page].size(); i++) {
    DisplayCurrentPageButtonRow(i, false);
  }

  // Page Footer
  switch(current_page) {
    case kSettingsPage:
      DisplayFirmwareVersionAndDate();
      break;
    case kWiFiSettingsPage:
      DisplayWiFiConnectionStatus();
      break;
    case kWeatherSettingsPage:
      DisplayWeatherInfo();
      break;
  }
}

void RGBDisplay::DisplayBlankScreen() {
  tft.fillScreen(kDisplayBackroundColor);
}

void RGBDisplay::DisplayWiFiConnectionStatus() {
  // clear any old text
  tft.setFont(&FreeSans12pt7b);
  tft.setTextColor(kDisplayBackroundColor);
  tft.setCursor(kDisplayTextGap, 190);
  tft.print("WiFi Connected!");

  tft.setFont(&FreeMono9pt7b);
  tft.setCursor(kDisplayTextGap, 170);
  tft.print("Could not");
  tft.setCursor(kDisplayTextGap, 190);
  tft.print("connect to saved");
  tft.setCursor(kDisplayTextGap, 210);
  tft.print("WiFi Network.");

  // write new text
  if(wifi_stuff->wifi_connected_) {
    tft.setFont(&FreeSans12pt7b);
    tft.setTextColor(kDisplayColorBlue);
    tft.setCursor(kDisplayTextGap, 190);
    tft.print("WiFi Connected!");
  }
  else {
    tft.setFont(&FreeMono9pt7b);
    tft.setTextColor(kDisplayColorBlue);
    tft.setCursor(kDisplayTextGap, 170);
    tft.print("Could not");
    tft.setCursor(kDisplayTextGap, 190);
    tft.print("connect to saved");
    tft.setCursor(kDisplayTextGap, 210);
    tft.print("WiFi Network.");
  }
}

void RGBDisplay::DisplayFirmwareVersionAndDate() {
  // Firmware Version and Date
  tft.setFont(&FreeMono9pt7b);
  tft.setTextColor(kDisplayColorBlue);
  tft.setCursor(10, kTftHeight - 20);
  tft.print("Firmware: ");
  tft.print(kFirmwareVersion.c_str());
  if(wifi_stuff->firmware_update_available_str_.size() > 0) {
    tft.setFont(&FreeMonoBold9pt7b);
    tft.print(" (latest)");
    tft.setFont(&FreeMono9pt7b);
  }
  tft.setCursor(10, kTftHeight - 5);
  tft.print("Date: ");
  tft.print(kFirmwareDate.c_str());
}

Cursor RGBDisplay::CheckButtonTouch() {
  int16_t ts_x = ts->GetTouchedPixel()->x, ts_y = ts->GetTouchedPixel()->y;
  Serial.print("ts_x:"); Serial.print(ts_x); Serial.print(", ts_y:"); Serial.println(ts_y);

  for (int i = 0; i < display_pages_vec[current_page].size(); i++) {
    DisplayButton* button = display_pages_vec[current_page][i];
    if(ts_x >= button->btn_x && ts_x <= button->btn_x + button->btn_w && ts_y >= button->btn_y && ts_y <= button->btn_y + button->btn_h) {
      //DisplayCurrentPageButtonRow(i, true);
      //delay(kUserInputDelayMs);
      return button->btn_cursor_id;
    }
  }

  // special case: check for touch input to WiFi Scan Networks List
  if(current_page == kWiFiScanNetworksPage) {
    int16_t cursorY0 = kWiFiScanNetworksList_y0_ - 0.75 * kWiFiScanNetworksList_h_ + 1;
    int16_t cursorYMax = kWiFiScanNetworksList_y0_ - 0.75 * kWiFiScanNetworksList_h_ + 1 + (kWiFiScanNetworksList_h_ * kWifiScanNetworksPageItems);
    // Serial.printf("cursorY0=%d, cursorYMax=%d\n", cursorY0, cursorYMax);
    if(ts_y >= cursorY0 && ts_y < cursorYMax) {
      // touch is inside the list
      // find touch box
      current_wifi_networks_scan_page_cursor = floor(float(ts_y - cursorY0) / kWiFiScanNetworksList_h_);
      // Serial.printf("current_wifi_networks_scan_page_cursor=%d\n", current_wifi_networks_scan_page_cursor);
      return kWiFiScanNetworksPageList;
    }
  }

  return kCursorNoSelection;
}

void RGBDisplay::RealTimeOnScreenOutput(std::string text, int width) {
  int kRectHeight = 20;
  tft.fillRect(0, 0, width, kRectHeight, kDisplayBackroundColor);
  tft.setFont(&FreeMono9pt7b);
  tft.setTextColor(kDisplayColorYellow);
  tft.setCursor(0, kRectHeight);
  tft.print(text.c_str());
}

void RGBDisplay::WiFiScanNetworksPage(bool increment_page) {

  tft.fillScreen(kDisplayBackroundColor);

  // Page Title
  tft.setFont(&FreeMonoBold9pt7b);
  tft.setTextColor(kDisplayColorGreen);
  tft.setCursor(kDisplayTextGap, 20);
  tft.print("SELECT WIFI NETWORK");

  tft.setTextColor(kDisplayColorYellow);
  tft.setFont(&FreeMono9pt7b);
  int cursorY = kWiFiScanNetworksList_y0_;

  // get number of WiFi Networks scanned
  int n_wifi_networks = wifi_stuff->WiFiScanNetworksCount();

  if(!increment_page) {
    current_wifi_networks_scan_page_no = 0;
  }
  else {
    current_wifi_networks_scan_page_no++;
    if(kWifiScanNetworksPageItems * current_wifi_networks_scan_page_no > n_wifi_networks - 1)
      current_wifi_networks_scan_page_no = 0;
  }
  current_wifi_networks_scan_page_cursor = 0;

  if(n_wifi_networks == 0) {
    tft.setCursor(10, cursorY);
    tft.print("No WiFi Networks found.");
    tft.setCursor(10, cursorY + 40);
    tft.print("Scan Again!");
  }
  else {
    // display WiFi scanned list
    for(int i = kWifiScanNetworksPageItems * current_wifi_networks_scan_page_no; i< min(kWifiScanNetworksPageItems * (current_wifi_networks_scan_page_no + 1), n_wifi_networks); i++) {
      tft.setCursor(10, cursorY);
      tft.print(wifi_stuff->WiFiScanNetworkDetails(i).c_str());
      cursorY += kWiFiScanNetworksList_h_;
    }
  }

  // Rescan button
  DrawButton(kRescanButtonX1, kRescanButtonY1, kRescanButtonW, kRescanButtonH, kRescanStr, kDisplayColorCyan, kDisplayColorOrange, kDisplayBackroundColor, true);

  // Next button
  DrawButton(kNextButtonX1, kNextButtonY1, kNextButtonW, kNextButtonH, kNextStr, kDisplayColorCyan, kDisplayColorOrange, kDisplayBackroundColor, true);

  // Back button
  DrawButton(kBackButtonX1, kBackButtonY1, kBackButtonW, kBackButtonH, kBackStr, kDisplayColorCyan, kDisplayColorOrange, kDisplayBackroundColor, true);
}

void RGBDisplay::SoftApInputsPage() {

  tft.fillScreen(kDisplayBackroundColor);

  // Page Title
  tft.setFont(&FreeMonoBold9pt7b);
  tft.setTextColor(kDisplayColorGreen);
  tft.setCursor(kDisplayTextGap, 20);
  tft.print("SET WIFI PASSWD USING MOBILE");

  tft.setTextColor(kDisplayColorYellow);
  tft.setFont(&FreeMono9pt7b);
  tft.setCursor(10, 50);
  //tft.print("A WiFi AP has been created.");
  tft.print("A WiFi Access Point has been");
  tft.setCursor(10, 70);
  // tft.print("using mobile/computer.");
  tft.print("created. Use your Mobile to");
  tft.setCursor(10, 90);
  // tft.print("WiFi SSID:");
  tft.print("connect to the WiFi:");

  tft.setCursor(10, 120);
  tft.setFont(&FreeSansBold12pt7b);
  tft.setTextColor(kDisplayColorGreen);
  tft.print(softApSsid);

  tft.setCursor(10, 150);
  tft.setTextColor(kDisplayColorYellow);
  tft.setFont(&FreeMono9pt7b);
  // tft.print("Open web browser and in");
  tft.print("Once connected, open a web");
  tft.setCursor(10, 170);
  // tft.print("address bar enter:");
  tft.print("browser and in address bar");

  tft.setCursor(10, 200);
  tft.print("enter:");

  tft.setCursor(80, 200);
  tft.setFont(&FreeSansBold12pt7b);
  tft.setTextColor(kDisplayColorGreen);
  tft.print(wifi_stuff->soft_AP_IP.c_str());

  tft.setCursor(10, 230);
  tft.setTextColor(kDisplayColorYellow);
  tft.setFont(&FreeMono9pt7b);
  // tft.print("Set your 2.4GHz");
  tft.print("& set WiFi Details.");
  // tft.setCursor(10, 220);
  // tft.print("WiFi details.");

  // Save button
  DrawButton(kSaveButtonX1, kSaveButtonY1, kSaveButtonW, kSaveButtonH, kSaveStr, kDisplayColorCyan, kDisplayColorOrange, kDisplayBackroundColor, true);

  // Back button
  DrawButton(kBackButtonX1, kBackButtonY1, kBackButtonW, kBackButtonH, kBackStr, kDisplayColorCyan, kDisplayColorOrange, kDisplayBackroundColor, true);
}

void RGBDisplay::LocationInputsLocalServerPage() {

  tft.fillScreen(kDisplayBackroundColor);

  // Page Title
  tft.setFont(&FreeMonoBold9pt7b);
  tft.setTextColor(kDisplayColorGreen);
  tft.setCursor(kDisplayTextGap, 20);
  tft.print("SET LOCATION USING MOBILE");

  tft.setTextColor(kDisplayColorYellow);
  tft.setFont(&FreeMono9pt7b);
  tft.setCursor(10, 50);
  tft.print("Have your mobile connected");
  tft.setCursor(10, 70);
  tft.print("to the following WiFi:");

  tft.setCursor(10, 100);
  tft.setFont(&FreeSansBold12pt7b);
  tft.setTextColor(kDisplayColorGreen);
  tft.print(wifi_stuff->wifi_ssid_.c_str());

  tft.setCursor(10, 130);
  tft.setTextColor(kDisplayColorYellow);
  tft.setFont(&FreeMono9pt7b);
  tft.print("Now open web browser and in");
  tft.setCursor(10, 150);
  tft.print("address bar enter:");

  tft.setCursor(10, 180);
  tft.setFont(&FreeSansBold12pt7b);
  tft.setTextColor(kDisplayColorGreen);
  tft.print(wifi_stuff->soft_AP_IP.c_str());

  tft.setCursor(10, 210);
  tft.setTextColor(kDisplayColorYellow);
  tft.setFont(&FreeMono9pt7b);
  tft.print("& set Location.");

  // Save button
  DrawButton(kSaveButtonX1, kSaveButtonY1, kSaveButtonW, kSaveButtonH, kSaveStr, kDisplayColorCyan, kDisplayColorOrange, kDisplayBackroundColor, true);

  // Back button
  DrawButton(kBackButtonX1, kBackButtonY1, kBackButtonW, kBackButtonH, kBackStr, kDisplayColorCyan, kDisplayColorOrange, kDisplayBackroundColor, true);
}

void RGBDisplay::DisplayWeatherInfo() {

  const int16_t alarm_triggered_page_title_y0 = 50;
  const int16_t long_press_alarm_seconds_y0 = alarm_triggered_page_title_y0 + 48;
  const int16_t city_x0 = 200;
  const int16_t city_y0 = alarm_triggered_page_title_y0 + 88;
  const int16_t weather_x0 = 5;
  const int16_t weather_main_y0 = long_press_alarm_seconds_y0 + 70;
  const int16_t weather_row2_y0 = weather_main_y0 + 25;
  const int16_t weather_row3_y0 = weather_row2_y0 + 20;
  const int16_t weather_row4_y0 = weather_row3_y0 + 20;

  // show today's weather
  if(wifi_stuff->got_weather_info_) {
    tft.setFont(&FreeSans12pt7b);
    tft.setTextColor(kDisplayColorPurple);
    tft.setCursor(city_x0, city_y0);
    tft.print(wifi_stuff->city_.c_str());

    tft.setCursor(weather_x0, weather_main_y0);
    tft.print(wifi_stuff->weather_main_.c_str()); tft.print(" : "); tft.print(wifi_stuff->weather_description_.c_str());

    tft.setFont(&FreeMono9pt7b);
    tft.setCursor(weather_x0, weather_row2_y0);
    tft.print("Temp: "); tft.print(wifi_stuff->weather_temp_.c_str()); tft.print("  Feels: "); tft.print(wifi_stuff->weather_temp_feels_like_.c_str());
    tft.setCursor(weather_x0, weather_row3_y0);
    tft.print("Max : "); tft.print(wifi_stuff->weather_temp_max_.c_str()); tft.print("  Min: "); tft.print(wifi_stuff->weather_temp_min_.c_str());
    tft.setCursor(weather_x0, weather_row4_y0);
    tft.print("Wind: "); tft.print(wifi_stuff->weather_wind_speed_.c_str()); tft.print(" Humidity: "); tft.print(wifi_stuff->weather_humidity_.c_str());
  }
  else {
    tft.setTextColor(kDisplayColorBlue);
    tft.setFont(&FreeMono9pt7b);
    if(wifi_stuff->openWeatherMapApiKey.size() == 0) {
      tft.setCursor(weather_x0, weather_row2_y0);
      tft.print("Cannot fetch/update time.");
      tft.setCursor(weather_x0, weather_row3_y0);
      tft.print("OpenWeatherMapApiKey empty!");
    }
    else if(wifi_stuff->get_weather_info_wait_seconds_ > 0) {
      tft.setCursor(weather_x0, weather_row2_y0);
      tft.print("Wait for ");
      tft.print(wifi_stuff->get_weather_info_wait_seconds_);
      tft.print(" seconds");
      tft.setCursor(weather_x0, weather_row3_y0);
      tft.print("before next Fetch.");
    }
    else {
      tft.setCursor(weather_x0, weather_row2_y0);
      tft.print("Could not fetch!");
      tft.setCursor(weather_x0, weather_row3_y0);
      tft.print(wifi_stuff->weather_fetch_error_message.c_str());
    }
  }
}

void RGBDisplay::FirmwareUpdatePage() {
  tft.fillScreen(kDisplayBackroundColor);
  tft.setTextColor(kDisplayColorYellow);
  tft.setFont(&FreeSans18pt7b);
  tft.setCursor(20, 80);
  tft.print("Updating Firmware");
  tft.setCursor(20, 140);
  tft.print("Over the Air..!");

  // Firmware Version and Date
  tft.setFont(&FreeMono9pt7b);
  tft.setTextColor(kDisplayColorYellow);
  tft.setCursor(10, kTftHeight - 20);
  tft.print("Active Firmware: ");
  tft.print(kFirmwareVersion.c_str());
  tft.setCursor(10, kTftHeight - 5);
  tft.print("Available Firmware: ");
  tft.print(wifi_stuff->firmware_update_available_str_.c_str());
}

void RGBDisplay::AlarmTriggeredScreen(bool firstTime, int8_t buttonPressSecondsCounter) {

  SetMaxBrightness();

  int16_t title_x0 = 0, title_y0 = 40;
  int16_t s_x0 = 220, s_y0 = title_y0 + 48;
  
  if(firstTime) {
    tft.fillScreen(kDisplayBackroundColor);
    tft.setFont(&Satisfy_Regular24pt7b);
    tft.setTextColor(kDisplayColorYellow);
    tft.setCursor(title_x0, title_y0);
    tft.print("WAKE");
    tft.setCursor(tft.getCursorX() + 5, title_y0);
    tft.print("UP!");
    tft.setFont(&FreeSans24pt7b);
    tft.setCursor(tft.getCursorX() + 5, title_y0);
    tft.setTextColor(kDisplayDateColor);
    if(rtc->hour() < 10)
      tft.print(kCharSpace);
    tft.print(new_display_data_.time_HHMM);
    tft.setFont(&FreeMonoBold9pt7b);
    int16_t cursor_x_ampmLabel = tft.getCursorX();
    tft.print('M');
    tft.setCursor(cursor_x_ampmLabel, title_y0 / 2);
    tft.print((new_display_data_.pm_not_am ? 'P' : 'A'));

    tft.setFont(&FreeMono9pt7b);
    tft.setTextColor(kDisplayColorCyan);
    char press_button_text1[] = "To turn off Alarm,";
    char press_button_text2[] = "press button for:";
    tft.setCursor(10, s_y0 - 18);
    tft.print(press_button_text1);
    tft.setCursor(10, s_y0);
    tft.print(press_button_text2);
    
    // show today's date
    tft.setCursor(15, s_y0 + 40);
    tft.setFont(&FreeSans18pt7b);
    tft.setTextColor(kDisplayDateColor);
    tft.print(new_display_data_.date_str);

    // show today's weather
    DisplayWeatherInfo();
  }

  char timer_str[6];
  int charIndex = 0;
  if(buttonPressSecondsCounter > 9) {
    timer_str[charIndex] = (char)(buttonPressSecondsCounter / 10 + 48); charIndex++;
  }
  timer_str[charIndex] = (char)(buttonPressSecondsCounter % 10 + 48); charIndex++;
  timer_str[charIndex] = 's'; charIndex++;
  timer_str[charIndex] = 'e'; charIndex++;
  timer_str[charIndex] = 'c'; charIndex++;
  timer_str[charIndex] = '\0';

  // fill rect
  tft.fillRect(s_x0 - 5, s_y0 - 40, kTftWidth - s_x0 + 5, 46, kDisplayBackroundColor);

  // set font
  tft.setFont(&Satisfy_Regular24pt7b);
  tft.setTextColor(kDisplayColorCyan);

  // home the cursor
  // uint16_t h = 0, w = 0;
  // int16_t x = 0, y = 0;
  // tft.getTextBounds(timer_str, 10, 150, &x, &y, &w, &h);
  // Serial.print("\nx "); Serial.print(x); Serial.print(" y "); Serial.print(y); Serial.print(" w "); Serial.print(w); Serial.print(" h "); Serial.println(h); 
  tft.setCursor(s_x0, s_y0);
  tft.print(timer_str);
}

void RGBDisplay::Screensaver() {
  const int16_t GAP_BAND = 5;
  if(refresh_screensaver_canvas_) {
    #ifdef MORE_LOGS
    // map time
    elapsedMillis timer1;
    #endif

    // delete created canvas and null the pointer
    if(my_canvas_ != NULL) {
      delete my_canvas_;
      my_canvas_ = NULL;
    }

    // get bounds of HH:MM text on screen
    tft.setFont(&ComingSoon_Regular70pt7b);
    tft.setTextColor(kDisplayBackroundColor);
    tft.getTextBounds(new_display_data_.time_HHMM, 0, 0, &gap_right_x_, &gap_up_y_, &tft_HHMM_w_, &tft_HHMM_h_);

    // get bounds of date string
    uint16_t date_h = 0, date_w = 0;
    int16_t date_gap_x = 0, date_gap_y = 0;
    if(rtc->hour() >= 10)
      tft.setFont(&FreeSans24pt7b);
    else
      tft.setFont(&FreeSans18pt7b);
    tft.getTextBounds(new_display_data_.date_str, 0, 0, &date_gap_x, &date_gap_y, &date_w, &date_h);
    
    int16_t date_x0 = GAP_BAND - date_gap_x;
    int16_t alarm_icon_w = (new_display_data_.alarm_ON ? kBellSmallWidth : kBellFallenSmallWidth);
    int16_t alarm_icon_h = (new_display_data_.alarm_ON ? kBellSmallHeight : kBellFallenSmallHeight);
    uint16_t date_row_w = date_w + 2 * GAP_BAND + alarm_icon_w;
    screensaver_w_ = max(tft_HHMM_w_ + 5 * GAP_BAND, date_row_w + 5 * GAP_BAND);
    screensaver_h_ = tft_HHMM_h_ + max((int)date_h, (int)alarm_icon_h) + 4*GAP_BAND;
    // middle both rows
    tft_HHMM_x0_ = (screensaver_w_ - tft_HHMM_w_) / 2 - gap_right_x_;
    date_x0 = (screensaver_w_ - date_row_w) / 2 - date_gap_x;
    
    // create canvas
    my_canvas_ = new GFXcanvas1(screensaver_w_, screensaver_h_);

    my_canvas_->setTextWrap(false);
    my_canvas_->fillScreen(kDisplayBackroundColor);

    // picknew random color
    if(!new_minute_)  // pick new color only when time hits top or bottom row, not when a minute change is there
      PickNewRandomColor();
    else
      new_minute_ = false;
    uint16_t randomColor = kColorPickerWheel[current_random_color_index_];

    // print HH:MM
    my_canvas_->setFont(&ComingSoon_Regular70pt7b);
    my_canvas_->setTextColor(randomColor);
    my_canvas_->setCursor(tft_HHMM_x0_ + GAP_BAND, GAP_BAND - gap_up_y_);
    my_canvas_->print(new_display_data_.time_HHMM);

    // print date string
    if(rtc->hour() >= 10)
      my_canvas_->setFont(&FreeSans24pt7b);
    else
      my_canvas_->setFont(&FreeSans18pt7b);
    my_canvas_->setTextColor(randomColor);
    my_canvas_->setCursor(date_x0 /*+ GAP_BAND */, screensaver_h_ - 4 * GAP_BAND);

    if(!firmware_updated_flag_user_information) {
      my_canvas_->print(new_display_data_.date_str);

      // draw bell
      my_canvas_->drawBitmap(my_canvas_->getCursorX() + 2*GAP_BAND, screensaver_h_ - alarm_icon_h - 3 * GAP_BAND, (new_display_data_.alarm_ON ? kBellSmallBitmap : kBellFallenSmallBitmap), alarm_icon_w, alarm_icon_h, randomColor);
    }
    else {
      my_canvas_->setFont(&FreeMonoBold9pt7b);
      // print firmware updated string
      std::string fw_updated_str = "Firmware Updated " + kFirmwareVersion + "!";
      my_canvas_->print(fw_updated_str.c_str());
    }

    // get visual bounds of created canvas and time string
    // myCanvas->drawRect(tft_HHMM_x0 + GAP_BAND, GAP_BAND, tft_HHMM_w, tft_HHMM_h, Display_Color_Green);  // time border
    // myCanvas->drawRect(date_x0 + GAP_BAND, screensaver_h + date_gap_y - 2 * GAP_BAND, date_row_w, date_h, Display_Color_Cyan);  // date row border
    if(show_colored_edge_screensaver_)
      my_canvas_->drawRect(0,0, screensaver_w_, screensaver_h_, kDisplayColorWhite);  // canvas border

    // re-center canvas if it is wider than tft width
    if(screensaver_bounce_not_fly_horizontally_ && screensaver_w_ >= kTftWidth) {
      screensaver_x1_ = ((int16_t)kTftWidth - (int16_t)screensaver_w_) / 2;
    }

    if(rtc->year() < 2024) {
      IncorrectTimeBanner();
    }

    // stop refreshing canvas until time change or if it hits top or bottom screen edges
    refresh_screensaver_canvas_ = false;

    #ifdef MORE_LOGS
    if(debug_mode) {
      unsigned long time1 = timer1;
      // Serial.printf("Screensave re-canvas time: %lums\n", time1);
      PrintLn("Screensave re-canvas time (ms): ", time1);
    }
    #endif
  }
  else {

    // move canvas on screen
    const int16_t kAdder = 1;
    const int16_t kHitLimit = 2;
    if(screensaver_bounce_not_fly_horizontally_) {
      if(!screensaver_move_right_ && screensaver_x1_ > -kHitLimit)
        screensaver_x1_ -= kAdder;
      else if(screensaver_move_right_ && screensaver_x1_ + screensaver_w_ < kTftWidth + 1 + kHitLimit)
        screensaver_x1_ += kAdder;
    }
    else {    // fly through right edge
      screensaver_x1_ += kAdder;
    }

    screensaver_y1_ += (screensaver_move_down_ ? kAdder : -kAdder);

    // set new direction on hitting any edge
    // left and right edge - only change direction
    // top and bottom edge - change direction and color
    if(!screensaver_move_right_ && screensaver_x1_ <= -kHitLimit) {   // left edge
      screensaver_move_right_ = true;
    }
    else if(screensaver_move_right_ && screensaver_x1_ + screensaver_w_ >= kTftWidth + 1 + kHitLimit) {    // right edge
      if(screensaver_bounce_not_fly_horizontally_) {
        screensaver_move_right_ = false;
      }
      else {  // fly through right edge and apprear back on left edge
        if(screensaver_x1_ >= kTftWidth)
          screensaver_x1_ = - kTftWidth;
      }
    }
    // top and bottom edge - when hit change color as well
    if(!screensaver_move_down_ && screensaver_y1_ <= -kHitLimit)  {   // top edge
      screensaver_move_down_ = true;
      refresh_screensaver_canvas_ = true;
    }
    else if(screensaver_move_down_ && screensaver_y1_ + screensaver_h_ >= kTftHeight + kHitLimit)  {   // bottom edge
      screensaver_move_down_ = false;
      refresh_screensaver_canvas_ = true;
    }
  }

  // paste the canvas on screen
  // tft.drawRGBBitmap(screensaver_x1, screensaver_y1, myCanvas->getBuffer(), screensaver_w, screensaver_h); // Copy to screen
  // tft.drawBitmap(screensaver_x1, screensaver_y1, myCanvas->getBuffer(), screensaver_w, screensaver_h, colorPickerWheelBright[currentRandomColorIndex], Display_Backround_Color); // Copy to screen
  FastDrawTwoColorBitmapSpi(screensaver_x1_, screensaver_y1_, my_canvas_->getBuffer(), screensaver_w_, screensaver_h_, kColorPickerWheel[current_random_color_index_], kDisplayBackroundColor);
  // // color LED Strip sequentially   ->   now done in loop1() by second core
}

void RGBDisplay::PickNewRandomColor() {
  int newIndex = current_random_color_index_;
  while(newIndex == current_random_color_index_)
    newIndex = random(0, kColorPickerWheelSize - 1);
  current_random_color_index_ = newIndex;
  #ifdef MORE_LOGS
  PrintLn("current_random_color_index_ = ", current_random_color_index_);
  #endif
}

void RGBDisplay::DisplayTimeUpdate() {

  bool isThisTheFirstTime = strcmp(displayed_data_.time_SS, "") == 0;
  if(redraw_display_) {
    tft.fillScreen(kDisplayBackroundColor);
    isThisTheFirstTime = true;
  }

  // initial gap if single digit hour
  const int16_t SINGLE_DIGIT_HOUR_GAP = 30;
  int16_t hh_gap_x = (rtc->hour() >= 10 ? 0 : SINGLE_DIGIT_HOUR_GAP);

  if(1) {   // CODE USES CANVAS AND ALWAYS PUTS HH:MM:SS AmPm on it every second

    // delete canvas if it exists
    if(my_canvas_ != NULL) {
      delete my_canvas_;
      my_canvas_ = NULL;
      // myCanvas.reset(nullptr);
    }

    // create new canvas for time row
    if(rtc->year() < 2024)  { // incorrect time
      my_canvas_ = new GFXcanvas1(kTftWidth, kTimeRowY0IncorrectTime);

      IncorrectTimeBanner();

      // draw canvas to tft   fastDrawBitmap
      FastDrawTwoColorBitmapSpi(0, 0, my_canvas_->getBuffer(), kTftWidth, kTimeRowY0IncorrectTime, kDisplayTimeColor, kDisplayBackroundColor); // Copy to screen
    }
    else {
      my_canvas_ = new GFXcanvas1(kTftWidth, kTimeRowY0 + 6);
      my_canvas_->fillScreen(kDisplayBackroundColor);
      my_canvas_->setTextWrap(false);

      // HH:MM

      // set font
      my_canvas_->setFont(&FreeSansBold48pt7b);

      // home the cursor
      my_canvas_->setCursor(kTimeRowX0 + hh_gap_x, kTimeRowY0);

      // change the text color to foreground color
      my_canvas_->setTextColor(kDisplayTimeColor);

      // draw the new time value
      my_canvas_->print(new_display_data_.time_HHMM);
      // tft.setTextSize(1);
      // delay(2000);

      // and remember the new value
      strcpy(displayed_data_.time_HHMM, new_display_data_.time_HHMM);


      // AM/PM

      int16_t x0_pos = my_canvas_->getCursorX();

      // set font
      my_canvas_->setFont(&FreeSans18pt7b);

      // draw new AM/PM
      if(new_display_data_._12_hour_mode) {

        // home the cursor
        my_canvas_->setCursor(x0_pos + kDisplayTextGap, kAM_PM_row_Y0);
        // Serial.print("tft_AmPm_x0 "); Serial.print(tft_AmPm_x0); Serial.print(" y0 "); Serial.print(tft_AmPm_y0); Serial.print(" tft.getCursorX() "); Serial.print(tft.getCursorX()); Serial.print(" tft.getCursorY() "); Serial.println(tft.getCursorY()); 

        // draw the new time value
        if(new_display_data_.pm_not_am)
          my_canvas_->print(kPmLabel);
        else
          my_canvas_->print(kAmLabel);
      }

      // and remember the new value
      displayed_data_._12_hour_mode = new_display_data_._12_hour_mode;
      displayed_data_.pm_not_am = new_display_data_.pm_not_am;


      // :SS

      // home the cursor
      my_canvas_->setCursor(x0_pos + kDisplayTextGap, kTimeRowY0);

      // draw the new time value
      my_canvas_->print(new_display_data_.time_SS);

      // and remember the new value
      strcpy(displayed_data_.time_SS, new_display_data_.time_SS);

      // draw canvas to tft   fastDrawBitmap
      FastDrawTwoColorBitmapSpi(0, 0, my_canvas_->getBuffer(), kTftWidth, kTimeRowY0 + 6, kDisplayTimeColor, kDisplayBackroundColor); // Copy to screen
    }

    // delete created canvas and null the pointer
    delete my_canvas_;
    my_canvas_ = NULL;
    // myCanvas.reset(nullptr);

  }
  else {    // CODE THAT CHECKS AND UPDATES ONLY CHANGES ON SCREEN HH:MM :SS AmPm

    // if new minute has come then clear the full time row and recreate it
    // GFXcanvas16* canvasPtr;
    if(rtc->second() == 0) {
      // canvasPtr = GFXcanvas16(TFT_WIDTH, TIME_ROW_Y0 + gap_up_y + tft_HHMM_h);
      // canvasPtr->fillScreen(Display_Backround_Color);
      tft.fillRect(0, 0, kTftWidth, kTimeRowY0 + gap_up_y_ + tft_HHMM_h_, kDisplayBackroundColor);
    }

    // HH:MM string and AM/PM string
    if (rtc->second() == 0 || strcmp(new_display_data_.time_HHMM, displayed_data_.time_HHMM) != 0 || redraw_display_) {

      // HH:MM

      // set font
      // tft.setTextSize(1);
      tft.setFont(&FreeSansBold48pt7b);

      // change the text color to the background color
      tft.setTextColor(kDisplayBackroundColor);

      // clear old time if it was there
      if(rtc->second() != 0 && !isThisTheFirstTime) {
        // home the cursor to currently displayed text location
        if(rtc->hour() == 10 && rtc->minute() == 0 && rtc->second() == 0)  // handle special case of moving from single digit hour to 2 digit hour while clearing old value
          tft.setCursor(kTimeRowX0 + SINGLE_DIGIT_HOUR_GAP, kTimeRowY0);
        else
          tft.setCursor(kTimeRowX0 + hh_gap_x, kTimeRowY0);

        // redraw the old value to erase
        tft.print(displayed_data_.time_HHMM);
        // tft.drawRect(tft_HHMM_x0 + gap_right_x, tft_HHMM_y0 + gap_up_y, tft_HHMM_w, tft_HHMM_h, Display_Color_White);
      }

      // get bounds of new HH:MM string on tft display (with background color as this causes a blink)
      tft.getTextBounds(new_display_data_.time_HHMM, 0, 0, &gap_right_x_, &gap_up_y_, &tft_HHMM_w_, &tft_HHMM_h_);
      // Serial.print("gap_right_x "); Serial.print(gap_right_x); Serial.print(" gap_up_y "); Serial.print(gap_up_y); Serial.print(" w "); Serial.print(tft_HHMM_w); Serial.print(" h "); Serial.println(tft_HHMM_h); 

      // home the cursor
      tft.setCursor(kTimeRowX0 + hh_gap_x, kTimeRowY0);
      // Serial.print("X0 "); Serial.print(TIME_ROW_X0); Serial.print(" Y0 "); Serial.print(TIME_ROW_Y0); Serial.print(" w "); Serial.print(tft_HHMM_w); Serial.print(" h "); Serial.println(tft_HHMM_h); 
      // tft.drawRect(TIME_ROW_X0 + gap_right_x, TIME_ROW_Y0 + gap_up_y, tft_HHMM_w, tft_HHMM_h, Display_Color_White);

      // change the text color to foreground color
      tft.setTextColor(kDisplayTimeColor);

      // draw the new time value
      tft.print(new_display_data_.time_HHMM);
      // tft.setTextSize(1);
      // delay(2000);

      // and remember the new value
      strcpy(displayed_data_.time_HHMM, new_display_data_.time_HHMM);

      // AM/PM

      // set font
      tft.setFont(&FreeSans18pt7b);

      // clear old AM/PM
      if(rtc->second() != 0 && !isThisTheFirstTime && displayed_data_._12_hour_mode) {
        // home the cursor
        tft.setCursor(tft_AmPm_x0_, tft_AmPm_y0_);

        // change the text color to the background color
        tft.setTextColor(kDisplayBackroundColor);

        // redraw the old value to erase
        if(displayed_data_.pm_not_am)
          tft.print(kPmLabel);
        else
          tft.print(kAmLabel);
      }

      // draw new AM/PM
      if(new_display_data_._12_hour_mode) {
        // set test location of Am/Pm
        tft_AmPm_x0_ = kTimeRowX0 + hh_gap_x + gap_right_x_ + tft_HHMM_w_ + 2 * kDisplayTextGap;
        tft_AmPm_y0_ = kTimeRowY0 + gap_up_y_ / 2;

        // home the cursor
        tft.setCursor(tft_AmPm_x0_, tft_AmPm_y0_);
        // Serial.print("tft_AmPm_x0 "); Serial.print(tft_AmPm_x0); Serial.print(" y0 "); Serial.print(tft_AmPm_y0); Serial.print(" tft.getCursorX() "); Serial.print(tft.getCursorX()); Serial.print(" tft.getCursorY() "); Serial.println(tft.getCursorY()); 

        // change the text color to the background color
        tft.setTextColor(kDisplayBackroundColor);

        // get bounds of new AM/PM string on tft display (with background color as this causes a blink)
        int16_t tft_AmPm_x1, tft_AmPm_y1;
        uint16_t tft_AmPm_w, tft_AmPm_h;
        tft.getTextBounds((new_display_data_.pm_not_am ? kPmLabel : kAmLabel), tft.getCursorX(), tft.getCursorY(), &tft_AmPm_x1, &tft_AmPm_y1, &tft_AmPm_w, &tft_AmPm_h);
        // Serial.print("AmPm_x1 "); Serial.print(tft_AmPm_x1); Serial.print(" y1 "); Serial.print(tft_AmPm_y1); Serial.print(" w "); Serial.print(tft_AmPm_w); Serial.print(" h "); Serial.println(tft_AmPm_h); 

        // calculate tft_AmPm_y0 to align top with HH:MM
        tft_AmPm_y0_ -= tft_AmPm_y1 - kTimeRowY0 - gap_up_y_;
        // Serial.print("tft_AmPm_y0 "); Serial.println(tft_AmPm_y0);

        // home the cursor
        tft.setCursor(tft_AmPm_x0_, tft_AmPm_y0_);

        // change the text color to foreground color
        tft.setTextColor(kDisplayTimeColor);

        // draw the new time value
        if(new_display_data_.pm_not_am)
          tft.print(kPmLabel);
        else
          tft.print(kAmLabel);
      }

      // and remember the new value
      displayed_data_._12_hour_mode = new_display_data_._12_hour_mode;
      displayed_data_.pm_not_am = new_display_data_.pm_not_am;
    }

    // :SS string
    if (rtc->second() == 0 || strcmp(new_display_data_.time_SS, displayed_data_.time_SS) != 0 || redraw_display_) {
      // set font
      tft.setFont(&FreeSans24pt7b);

      // clear old seconds
      if(rtc->second() != 0 && !isThisTheFirstTime) {
        // change the text color to the background color
        tft.setTextColor(kDisplayBackroundColor);

        // home the cursor
        tft.setCursor(tft_SS_x0_, kTimeRowY0);

        // change the text color to the background color
        tft.setTextColor(kDisplayBackroundColor);

        // redraw the old value to erase
        tft.print(displayed_data_.time_SS);
      }

      // fill new home values
      tft_SS_x0_ = kTimeRowX0 + hh_gap_x + gap_right_x_ + tft_HHMM_w_ + kDisplayTextGap;

      // home the cursor
      tft.setCursor(tft_SS_x0_, kTimeRowY0);

      // change the text color to foreground color
      tft.setTextColor(kDisplayTimeColor);

      // draw the new time value
      tft.print(new_display_data_.time_SS);

      // and remember the new value
      strcpy(displayed_data_.time_SS, new_display_data_.time_SS);
    }
  }

  // date string center aligned
  if (strcmp(new_display_data_.date_str, displayed_data_.date_str) != 0 || redraw_display_) {
    if(rtc->year() < 2024) {
      // if time is incorrect then don't bother drawing date row

      // draw settings gear
      tft.drawBitmap(kSettingsGearX1, kSettingsGearY1, kSettingsGearBitmap, kSettingsGearWidth, kSettingsGearHeight, kDisplayGearIconColor); // Copy to screen
    }
    else {
      // set font
      tft.setFont(&Satisfy_Regular24pt7b);

      // change the text color to the background color
      tft.setTextColor(kDisplayBackroundColor);

      // clear old data
      if(!isThisTheFirstTime) {
        // yes! home the cursor
        tft.setCursor(date_row_x0_, kDateRow_Y0);

        // redraw the old value to erase
        tft.print(displayed_data_.date_str);
      }

      // home the cursor
      tft.setCursor(date_row_x0_, kDateRow_Y0);

      // record date_row_w to calculate center aligned date_row_x0 value
      int16_t date_row_y1;
      uint16_t date_row_w, date_row_h;
      // get bounds of new dateStr on tft display (with background color as this causes a blink)
      tft.getTextBounds(new_display_data_.date_str, tft.getCursorX(), tft.getCursorY(), &date_row_x0_, &date_row_y1, &date_row_w, &date_row_h);
      date_row_x0_ = (kSettingsGearX1 - date_row_w) / 2;

      // home the cursor
      tft.setCursor(date_row_x0_, kDateRow_Y0);

      // change the text color to foreground color
      tft.setTextColor(kDisplayDateColor);

      // draw the new dateStr value
      tft.print(new_display_data_.date_str);

      // draw settings gear
      tft.drawBitmap(kSettingsGearX1, kSettingsGearY1, kSettingsGearBitmap, kSettingsGearWidth, kSettingsGearHeight, kDisplayGearIconColor); // Copy to screen

      // and remember the new value
      strcpy(displayed_data_.date_str, new_display_data_.date_str);
    }
  }

  // alarm string center aligned
  if (strcmp(new_display_data_.alarm_str, displayed_data_.alarm_str) != 0 || new_display_data_.alarm_ON != displayed_data_.alarm_ON || redraw_display_) {
    // set font
    tft.setFont(&Satisfy_Regular24pt7b);

    int16_t alarm_icon_w, alarm_icon_h;

    // change the text color to the background color
    tft.setTextColor(kDisplayBackroundColor);

    // clear old data
    if(!isThisTheFirstTime) {
      // yes! home the cursor
      tft.setCursor(alarm_row_x0_, kAlarmRowY0);

      // redraw the old value to erase
      tft.print(displayed_data_.alarm_str);

      if(displayed_data_.alarm_ON) {
        alarm_icon_w = kBellWidth;
        alarm_icon_h = kBellHeight;
      }
      else {
        alarm_icon_w = kBellFallenWidth;
        alarm_icon_h = kBellFallenHeight;
      }

      // erase bell
      tft.drawBitmap(alarm_icon_x0_, alarm_icon_y0_, (displayed_data_.alarm_ON ? kBellBitmap : kBellFallenBitmap), alarm_icon_w, alarm_icon_h, kDisplayBackroundColor);
    }

    //  Redraw new alarm data

    if(new_display_data_.alarm_ON) {
      alarm_icon_w = kBellWidth;
      alarm_icon_h = kBellHeight;
    }
    else {
      alarm_icon_w = kBellFallenWidth;
      alarm_icon_h = kBellFallenHeight;
    }

    // home the cursor
    tft.setCursor(alarm_icon_x0_, kAlarmRowY0);

    // highlight background while showing change log and firmware update info
    if(firmware_updated_flag_user_information)
      tft.fillRect(0, kDateRow_Y0 + 10, kTftWidth, kTftHeight - kDateRow_Y0 - 10, kDisplayColorGrey);

    // record alarm_row_w to calculate center aligned alarm_row_x0 value
    int16_t alarm_row_y1;
    uint16_t alarm_row_w, alarm_row_h;
    // get bounds of new alarmStr on tft display (with background color as this causes a blink)
    tft.getTextBounds(new_display_data_.alarm_str, tft.getCursorX(), tft.getCursorY(), &alarm_row_x0_, &alarm_row_y1, &alarm_row_w, &alarm_row_h);
    uint16_t graphic_width = alarm_icon_w + alarm_row_w;
    // three equal length gaps on left center and right of graphic
    uint16_t equal_gaps = (kTftWidth - graphic_width) / 3;
    alarm_row_x0_ = equal_gaps + alarm_icon_w + equal_gaps;
    alarm_icon_x0_ = equal_gaps;
    // align bell at bottom of screen
    alarm_icon_y0_ = kTftHeight - alarm_icon_h;

    // home the cursor
    tft.setCursor(alarm_row_x0_, kAlarmRowY0);

    // change the text color to foreground color
    tft.setTextColor(kDisplayAlarmColor);

    // draw the new alarmStr value
    tft.print(new_display_data_.alarm_str);

    // draw bell
    tft.drawBitmap(alarm_icon_x0_, alarm_icon_y0_, (new_display_data_.alarm_ON ? kBellBitmap : kBellFallenBitmap), alarm_icon_w, alarm_icon_h, kDisplayAlarmColor);

    // and remember the new value
    strcpy(displayed_data_.alarm_str, new_display_data_.alarm_str);
    displayed_data_.alarm_ON = new_display_data_.alarm_ON;
  }

  if(redraw_display_ && firmware_updated_flag_user_information) {
    // set font
    tft.setFont(&FreeSansBold12pt7b);
    // color
    tft.setTextColor(kDisplayColorBlue);
    // home the cursor
    tft.setCursor(alarm_icon_x0_, alarm_icon_y0_ + 10);
    // print firmware updated string
    std::string fw_updated_str = "Firmware Updated " + kFirmwareVersion + "!";
    tft.print(fw_updated_str.c_str());
    // color for changelog
    tft.setCursor(0, alarm_icon_y0_ + 30);
    // tft.setTextColor(kDisplayColorBlue);
    tft.setFont(&FreeMonoBold9pt7b);
    tft.print(kChangeLog.c_str());
  }

  redraw_display_ = false;
}

void RGBDisplay::IncorrectTimeBanner() {
  // RTC Time is not Set!
  my_canvas_->fillRect(0, 0, kTftWidth, kTimeRowY0IncorrectTime, kDisplayBackroundColor);
  my_canvas_->drawRect(0, 0, kTftWidth, kTimeRowY0IncorrectTime, kDisplayTimeColor);
  // my_canvas_->setTextColor(kDisplayTimeColor);
  my_canvas_->setFont(&FreeSansBold12pt7b);
  my_canvas_->setCursor(kDisplayTextGap, 30);
  my_canvas_->print("Incorrect Time!");
  my_canvas_->setFont(&FreeMono9pt7b);
  my_canvas_->setCursor(kDisplayTextGap, 50);
  my_canvas_->print("Power failure! Battery Down!");
  my_canvas_->setCursor(kDisplayTextGap, 70);
  my_canvas_->print("Time Update Required!");
  my_canvas_->setCursor(kDisplayTextGap, 90);
  if(!(wifi_stuff->incorrect_wifi_details_) && !(wifi_stuff->incorrect_zip_code))
    my_canvas_->print("Updating Time using WiFi..");
  else if(wifi_stuff->incorrect_wifi_details_)
    my_canvas_->print("Could not connect to WiFi.");
  else if(wifi_stuff->weather_fetch_error_message.size() > 0)
    my_canvas_->print(wifi_stuff->weather_fetch_error_message.c_str());
}

void RGBDisplay::ButtonHighlight(int16_t x, int16_t y, uint16_t w, uint16_t h, bool turnOn, int gap) {
  if(turnOn)
    tft.drawRoundRect(x - gap, y - 2 * gap, w + 3 * gap, h + 4 * gap, kRadiusButtonRoundRect, kDisplayColorCyan);
  else
    tft.drawRoundRect(x - gap, y - 2 * gap, w + 3 * gap, h + 4 * gap, kRadiusButtonRoundRect, kDisplayBackroundColor);
}

void RGBDisplay::GoodMorningScreen() {
  tft.fillScreen(kDisplayBackroundColor);

  std::string owner_name;
  nvs_preferences->RetrieveOwnerName(owner_name);
  std::string good_morning_str = "GOOD MORNING";

  // get bounds of good morning text on screen
  tft.setFont(&FreeSans18pt7b);
  tft.setTextColor(kDisplayBackroundColor);
  uint16_t good_morning_str_h = 0, good_morning_str_w = 0;
  int16_t good_morning_str_gap_x = 0, good_morning_str_gap_y = 0;
  tft.getTextBounds(good_morning_str.c_str(), 0, 0, &good_morning_str_gap_x, &good_morning_str_gap_y, &good_morning_str_w, &good_morning_str_h);

  // get bounds of owner name text on screen
  if(owner_name.length() <= 10)
    tft.setFont(&FreeSans24pt7b);
  else
  tft.setFont(&FreeSans18pt7b);
  uint16_t owner_name_h = 0, owner_name_w = 0;
  int16_t owner_name_gap_x = 0, owner_name_gap_y = 0;
  tft.getTextBounds(owner_name.c_str(), 0, 0, &owner_name_gap_x, &owner_name_gap_y, &owner_name_w, &owner_name_h);

  // change the text color to the background color
  tft.setTextColor(kDisplayColorGreen);

  // set font
  tft.setFont(&FreeSans18pt7b);
  // yes! home the cursor
  tft.setCursor((kTftWidth - good_morning_str_w) / 2, good_morning_str_h);
  // redraw the old value to erase
  tft.print(good_morning_str.c_str());
  
  // set font
  if(owner_name.length() <= 10)
    tft.setFont(&FreeSans24pt7b);
  else
    tft.setFont(&FreeSans18pt7b);
  // yes! home the cursor
  tft.setCursor((kTftWidth - owner_name_w) / 2, good_morning_str_h + owner_name_h + 5);
  // redraw the old value to erase
  tft.print(owner_name.c_str());

  // draw sun
  uint16_t edge = kTftHeight - (good_morning_str_h + owner_name_h);
  int16_t x0 = (kTftWidth - edge) / 2, y0 = (good_morning_str_h + owner_name_h) + 5;
  
  unsigned int startTime = millis();

  // start tone
  int tone_note_index = 0;
  unsigned long next_tone_change_time = startTime;
  alarm_clock->celebrateSong(tone_note_index, next_tone_change_time);

  while(millis() - startTime < 5000)
    DrawSun(x0, y0, edge, tone_note_index, next_tone_change_time);

  tft.fillScreen(kDisplayBackroundColor);
  redraw_display_ = true;
}

/* draw Sun
 * 
 * params: top left corner 'x0' and 'y0', square edge length of graphic 'edge'
 */ 
void RGBDisplay::DrawSun(int16_t x0, int16_t y0, uint16_t edge, int &tone_note_index, unsigned long &next_tone_change_time) {

  // set dimensions of sun and rays

  // sun center
  int16_t cx = x0 + edge / 2, cy = y0 + edge / 2;
  // sun radius
  int16_t sr = edge * 0.23;
  // length of rays
  int16_t rl = edge * 0.09;
  // rays inner radius
  int16_t rr = sr + edge * 0.08;
  // width of rays
  int16_t rw = 5;
  // number of rays
  uint8_t rn = 12;

  // color
  uint16_t color = kDisplayColorYellow;
  uint16_t background = kDisplayBackroundColor;

  int16_t variation_prev = 0;

  // sun
  tft.fillCircle(cx, cy, sr, color);

  // eyes
  int16_t eye_offset_x = sr / 2, eye_offset_y = sr / 3, eye_r = max(sr / 8, 3);
  tft.fillCircle(cx - eye_offset_x, cy - eye_offset_y, eye_r, background);
  tft.fillCircle(cx + eye_offset_x, cy - eye_offset_y, eye_r, background);

  // draw smile
  int16_t smile_angle_deg = 37;
  int16_t smile_cy = cy - sr / 2;
  int16_t smile_r = sr * 1.1, smile_w = max(sr / 15, 3);
  for(uint8_t i = 0; i <= smile_angle_deg; i=i+2) {
    float smile_angle_rad = DEG_TO_RAD * i;
    int16_t smile_tapered_w = max(smile_w - i / 13, 1);
    // Serial.print(i); Serial.print(" "); Serial.print(smile_w); Serial.print(" "); Serial.println(smile_tapered_w);
    int16_t smile_offset_x = smile_r * sin(smile_angle_rad), smile_offset_y = smile_r * cos(smile_angle_rad);
    tft.fillCircle(cx - smile_offset_x, smile_cy + smile_offset_y, smile_tapered_w, background);
    tft.fillCircle(cx + smile_offset_x, smile_cy + smile_offset_y, smile_tapered_w, background);
  }

  // draw changing rays
  for(int16_t i = 0; i < 120; i++) {
    ResetWatchdog();
    // variation goes from 0 to 5 to 0
    int16_t i_base10_fwd = i % 10;
    int16_t i_base10_bwd = ((i / 10) + 1) * 10 - i;
    int16_t variation = min(i_base10_fwd, i_base10_bwd);
    // Serial.println(variation);
    int16_t r_variable = rr + variation;
    // draw rays
    DrawRays(cx, cy, r_variable, rl, rw, rn, i, color);
    // increase sun size
    // tft.drawCircle(cx, cy, sr + variation, color);
    DrawDenseCircle(cx, cy, sr + variation, color);
    // show for sometime
    delay(30);

    // undraw rays
    DrawRays(cx, cy, r_variable, rl, rw, rn, i, background);
    // reduce sun size
    if(variation < variation_prev){
      // tft.drawCircle(cx, cy, sr + variation_prev, background);
      DrawDenseCircle(cx, cy, sr + variation_prev + 1, background);
    }
    // delay(1000);
    variation_prev = variation;

    // celebration tone
    if(millis() > next_tone_change_time)
      alarm_clock->celebrateSong(tone_note_index, next_tone_change_time);
  }
}

/* draw rays
 * 
 * params: center cx, cy; inner radius of rays rr, length of rays rl, width of rays rw, number of rays rn, start angle degStart, color
 */ 
void RGBDisplay::DrawRays(int16_t &cx, int16_t &cy, int16_t &rr, int16_t &rl, int16_t &rw, uint8_t &rn, int16_t &degStart, uint16_t &color) {
  // rays
  for(uint8_t i = 0; i < rn; i++) {
    // find coordinates of two triangles for each ray and use fillTriangle function to draw rays
    float theta = 2 * PI * i / rn + degStart * DEG_TO_RAD;
    double rcos = rr * cos(theta), rlcos = (rr + rl) * cos(theta), rsin = rr * sin(theta), rlsin = (rr + rl) * sin(theta);
    double w2sin = rw / 2 * sin(theta), w2cos = rw / 2 * cos(theta);
    int16_t x1 = cx + rcos - w2sin;
    int16_t x2 = cx + rcos + w2sin;
    int16_t x3 = cx + rlcos + w2sin;
    int16_t x4 = cx + rlcos - w2sin;
    int16_t y1 = cy + rsin + w2cos;
    int16_t y2 = cy + rsin - w2cos;
    int16_t y3 = cy + rlsin - w2cos;
    int16_t y4 = cy + rlsin + w2cos;
    tft.fillTriangle(x1, y1, x2, y2, x3, y3, color);
    tft.fillTriangle(x1, y1, x3, y3, x4, y4, color);
  }
}

/* drawDenseCircle
 * densely pack a circle's circumference
 */
void RGBDisplay::DrawDenseCircle(int16_t &cx, int16_t &cy, int16_t r, uint16_t &color) {
  // calculate angular resolution required
  // r*dTheta = 0.5
  double dTheta = 0.5 / static_cast<double>(r);
  // number of runs to cover quarter circle
  uint32_t n = PI / 2 / dTheta;
  // Serial.print("dTheta "); Serial.print(dTheta, 5); Serial.print(" n "); Serial.println(n);

  for(uint32_t i = 0; i < n; i++) {
    float theta = i * dTheta; // Serial.print(" theta "); Serial.println(theta);
    int16_t rcos = r * cos(theta);
    int16_t rsin = r * sin(theta);
    tft.drawPixel(cx + rcos, cy + rsin, color);
    tft.drawPixel(cx - rcos, cy + rsin, color);
    tft.drawPixel(cx + rcos, cy - rsin, color);
    tft.drawPixel(cx - rcos, cy - rsin, color);
  }
}

// make keyboard on screen

void RGBDisplay::DrawKeyboardButton(TouchKbKeys kb_key_flag, bool clicked, int key_array_x = 0, int key_array_y = 0, int cursor_shift_right = 0, char letter = 0) {

  int x = 0, y = 0, w = 0, h = 0;
  bool on = false;
  char* label = nullptr;

  GetKeyBoardKeyDimensions(x, y, w, h, kb_key_flag, key_array_x, key_array_y, cursor_shift_right);

  // key labels
  switch(kb_key_flag) 
  {
    case KB_ALPHANUMERIC_KEY:
      break;
    case KB_DELETE_KEY:
      label = "DEL";
      break;
    case KB_ENTER_KEY:
      label = "ENTER";
      break;
    case KB_SHIFT_KEY:
      if(GetKeyboardPress_numpad)
        label = "SPECIAL";
      else
        label = "CAPITAL";
      on = GetKeyboardPress_shift;
      break;
    case KB_SPACEBAR_KEY:
      label = "SPACE BAR";
      break;
    case KB_NUMPAD_KEY:
      label = "NUMPAD";
      on = GetKeyboardPress_numpad;
      break;
    case KB_BACK_BUTTON:
      label = (char *)kBackStr;
      break;
  }
  
  // draw key

  if(!clicked) {
    // button outside shadow
    tft.fillRoundRect(x - 3, y + 3, w, h, 3, 0x8888);    
  }
  else {
    // push button down
    x -= 1;
    y += 1;
  }

  // button border
  tft.fillRoundRect(x, y, w, h, 3, 0xffff);

  // button inside fill
  tft.fillRoundRect(x + 1, y + 1, w - 1 * 2, h - 1 * 2, 3, (clicked ? kButtonClickedFillColor : (on ? kTextHighLightColor : kKeyboardButtonFillColor)));

  // button label
  tft.setCursor(x + (w < 100 ? 4 : 18), y + h/5);
  tft.setTextColor((on ? kDisplayBackroundColor : kTextRegularColor));
  if(label == nullptr) {
    tft.print(letter);
  }
  else {
    tft.print(label);
  }
}

bool RGBDisplay::IsTouchWithin(TouchKbKeys kb_key_flag, int key_array_x = 0, int key_array_y = 0, int cursor_shift_right = 0) {
  int x = 0, y = 0, w = 0, h = 0;
  GetKeyBoardKeyDimensions(x, y, w, h, kb_key_flag, key_array_x, key_array_y, cursor_shift_right);

  // tft.fillCircle(x, y, 2, 0x0FF0);
  return ((((ts->GetTouchedPixel())->x>=x)&&((ts->GetTouchedPixel())->x<=x + w)) & (((ts->GetTouchedPixel())->y>=y)&&((ts->GetTouchedPixel())->y<=y + h)));
}

void RGBDisplay::GetKeyBoardKeyDimensions(int &x, int &y, int &w, int &h, TouchKbKeys kb_key_flag, int key_array_x = 0, int key_array_y = 0, int cursor_shift_right = 0) {
  switch(kb_key_flag) 
  {
    case KB_ALPHANUMERIC_KEY:
      x = 8 + (23 * (key_array_x - 3)) + cursor_shift_right, y = kTextAreaHeight + (30 * key_array_y), w = KB_ALPHANUMERIC_KEY_W, h = KB_ALL_KEY_H;
      break;
    case KB_DELETE_KEY:
      x = KB_DELETE_KEY_X, y = KB_DELETE_KEY_Y, w = KB_DELETE_KEY_W, h = KB_ALL_KEY_H;
      break;
    case KB_ENTER_KEY:
      x = KB_ENTER_KEY_X, y = KB_ENTER_KEY_Y, w = KB_ENTER_KEY_W, h = KB_ALL_KEY_H;
      break;
    case KB_SHIFT_KEY:
      x = KB_SHIFT_KEY_X, y = KB_SHIFT_KEY_Y, w = KB_SHIFT_KEY_W, h = KB_ALL_KEY_H;
      break;
    case KB_SPACEBAR_KEY:
      x = KB_SPACEBAR_KEY_X, y = KB_SPACEBAR_KEY_Y, w = KB_SPACEBAR_KEY_W, h = KB_ALL_KEY_H;
      break;
    case KB_NUMPAD_KEY:
      x = KB_NUMPAD_KEY_X, y = KB_NUMPAD_KEY_Y, w = KB_NUMPAD_KEY_W, h = KB_ALL_KEY_H;
      break;
    case KB_BACK_BUTTON:
      x = kBackButtonX1, y = kBackButtonY1, w = kBackButtonW, h = kBackButtonH;
      break;
  }
}

// credits: Andrew Mascolo https://github.com/AndrewMascolo/Adafruit_Stuff/blob/master/Sketches/Keyboard.ino
void RGBDisplay::MakeKeyboard(const char type[][13], std::string label) {
  // heading label
  tft.setTextSize(1);
  tft.setFont(&FreeMono9pt7b);
  tft.setCursor(0, 12);
  tft.setTextColor(kTextRegularColor, kDisplayBackroundColor);
  tft.print(label.c_str());

  // other text font
  tft.setFont(NULL);
  tft.setTextSize(2);

  // keys
  tft.setTextColor(kTextRegularColor, kKeyboardButtonFillColor);
  for (int y = 0; y < (kb_numbers_only ? 1 : 3); y++) {
    int cursor_shift_right = 10 * pgm_read_byte(&(type[y][0]));
    for (int x = 3; x < 13; x++) {
      if (x >= pgm_read_byte(&(type[y][1]))) break;
      DrawKeyboardButton(KB_ALPHANUMERIC_KEY, /*bool clicked*/ false, x, y, cursor_shift_right, char(pgm_read_byte(&(type[y][x]))));
    }
  }

  if(!kb_numbers_only && !kb_alphabets_only) {
    DrawKeyboardButton(KB_SHIFT_KEY, /*bool clicked*/ false);
    DrawKeyboardButton(KB_NUMPAD_KEY, /*bool clicked*/ false);
    DrawKeyboardButton(KB_SPACEBAR_KEY, /*bool clicked*/ false);
  }
  DrawKeyboardButton(KB_DELETE_KEY, /*bool clicked*/ false);
  DrawKeyboardButton(KB_ENTER_KEY, /*bool clicked*/ false);
  DrawKeyboardButton(KB_BACK_BUTTON, /*bool clicked*/ false);
}

// get keyboard presses on keyboard made by MakeKeyboard
// credits: Andrew Mascolo https://github.com/AndrewMascolo/Adafruit_Stuff/blob/master/Sketches/Keyboard.ino
bool RGBDisplay::GetKeyboardPress(char * textBuffer, std::string label, char * textReturn) {
  //// static vars for Keyboard
  //static bool GetKeyboardPress_shift = true, GetKeyboardPress_lastShift = true, GetKeyboardPress_numpad = false, GetKeyboardPress_lastNumpad = false;
  static char bufIndex = 0;
  // increase bufIndex to length of pre-filled user input
  while(textBuffer[bufIndex] != '\0')
    bufIndex++;

  if (ts->IsTouched())
  {
    // check if back button is pressed
    if (IsTouchWithin(KB_BACK_BUTTON))
    {
      DrawKeyboardButton(KB_BACK_BUTTON, /*bool clicked*/ true);
      delay(kUserInputDelayMs);
      DrawKeyboardButton(KB_BACK_BUTTON, /*bool clicked*/ false);
      return false;
    }

    // ShiftKey
    if (!kb_numbers_only && !kb_alphabets_only && IsTouchWithin(KB_SHIFT_KEY))
    {
      DrawKeyboardButton(KB_SHIFT_KEY, /*bool clicked*/ true);
      delay(kUserInputDelayMs);
      DrawKeyboardButton(KB_SHIFT_KEY, /*bool clicked*/ false);

      GetKeyboardPress_shift = !GetKeyboardPress_shift;
    }

    // Numpad
    if (!kb_numbers_only && !kb_alphabets_only && IsTouchWithin(KB_NUMPAD_KEY))
    {
      DrawKeyboardButton(KB_NUMPAD_KEY, /*bool clicked*/ true);
      delay(kUserInputDelayMs);
      DrawKeyboardButton(KB_NUMPAD_KEY, /*bool clicked*/ false);

      GetKeyboardPress_numpad = !GetKeyboardPress_numpad;
    }

    // re-draw keyboard if...
    if (GetKeyboardPress_numpad != GetKeyboardPress_lastNumpad || GetKeyboardPress_shift != GetKeyboardPress_lastShift)
    {
      if (GetKeyboardPress_numpad)
      {
        if (GetKeyboardPress_shift)
        {
          tft.fillScreen(kDisplayBackroundColor);
          MakeKeyboard(Mobile_SymKeys, label);
        }
        else
        {
          tft.fillScreen(kDisplayBackroundColor);
          MakeKeyboard(Mobile_NumKeys, label);
        }
      }
      else
      {
        if (GetKeyboardPress_shift)
        {
          tft.fillScreen(kDisplayBackroundColor);
          MakeKeyboard(Mobile_KB_Capitals, label);
          tft.setTextColor(kTextRegularColor, kKeyboardButtonFillColor);
        }
        else
        {
          tft.fillScreen(kDisplayBackroundColor);
          MakeKeyboard(Mobile_KB_Smalls, label);
          tft.setTextColor(kTextRegularColor, kKeyboardButtonFillColor);
        }
      }

      DrawKeyboardButton(KB_NUMPAD_KEY, /*bool clicked*/ false);
      DrawKeyboardButton(KB_SHIFT_KEY, /*bool clicked*/ false);

      GetKeyboardPress_lastNumpad = GetKeyboardPress_numpad;
      GetKeyboardPress_lastShift = GetKeyboardPress_shift;
    }

    for (int y = 0; y < (kb_numbers_only ? 1 : 3); y++)
    {
      int cursor_shift_right;
      if (GetKeyboardPress_numpad)
      {
        if (GetKeyboardPress_shift)
          cursor_shift_right = 10 * pgm_read_byte(&(Mobile_SymKeys[y][0]));
        else
          cursor_shift_right = 10 * pgm_read_byte(&(Mobile_NumKeys[y][0]));
      }
      else
      {
        if (GetKeyboardPress_shift)
          cursor_shift_right = 10 * pgm_read_byte(&(Mobile_KB_Capitals[y][0]));
        else
          cursor_shift_right = 10 * pgm_read_byte(&(Mobile_KB_Smalls[y][0]));
      }

      for (int x = 3; x < 13; x++)
      {
        if (x >=  (GetKeyboardPress_numpad ? (GetKeyboardPress_shift ? pgm_read_byte(&(Mobile_SymKeys[y][1])) : pgm_read_byte(&(Mobile_NumKeys[y][1]))) : pgm_read_byte(&(Mobile_KB_Capitals[y][1])) ))
          break;

        if (IsTouchWithin(KB_ALPHANUMERIC_KEY, x, y, cursor_shift_right))
        {// this will draw the button on the screen by so many pixels
          if (bufIndex < (kWifiSsidPasswordLengthMax))
          {
            char pressed_char = 0;

            if (GetKeyboardPress_numpad) {
              if (GetKeyboardPress_shift)
                pressed_char = pgm_read_byte(&(Mobile_SymKeys[y][x]));
              else
                pressed_char = pgm_read_byte(&(Mobile_NumKeys[y][x]));
            }
            else {
              if (GetKeyboardPress_shift)
                pressed_char = pgm_read_byte(&(Mobile_KB_Capitals[y][x]));
              else
                pressed_char = pgm_read_byte(&(Mobile_KB_Smalls[y][x]));
            }
            //pressed_char = (pgm_read_byte(&(Mobile_KB_Capitals[y][x])) + (GetKeyboardPress_shift ? 0 : ('a' - 'A')));

            // draw pressed key
            DrawKeyboardButton(KB_ALPHANUMERIC_KEY, /*bool clicked*/ true, x, y, cursor_shift_right, pressed_char);
            delay(kUserInputDelayMs);

            textBuffer[bufIndex] = pressed_char;

            // draw key
            DrawKeyboardButton(KB_ALPHANUMERIC_KEY, /*bool clicked*/ false, x, y, cursor_shift_right, pressed_char);

            bufIndex++;
          }
          break;
        }
      }
    }

    // Spacebar
    if (!kb_numbers_only && !kb_alphabets_only && IsTouchWithin(KB_SPACEBAR_KEY))
    {
      textBuffer[bufIndex++] = ' ';

      DrawKeyboardButton(KB_SPACEBAR_KEY, /*bool clicked*/ true);
      delay(kUserInputDelayMs);
      DrawKeyboardButton(KB_SPACEBAR_KEY, /*bool clicked*/ false);
    }

    // Delete / BackSpace
    if (IsTouchWithin(KB_DELETE_KEY))
    {
      if ((bufIndex) > 0)
        bufIndex--;
      textBuffer[bufIndex] = 0;
      // clear writing pad
      tft.fillRect(15, kTextAreaHeight - 30, tft.width() - 40, 20, kDisplayBackroundColor);

      DrawKeyboardButton(KB_DELETE_KEY, /*bool clicked*/ true);
      delay(kUserInputDelayMs);
      DrawKeyboardButton(KB_DELETE_KEY, /*bool clicked*/ false);
    }

    // Enter
    if (IsTouchWithin(KB_ENTER_KEY))
    {
      // Serial.println(textBuffer);
      strcpy(textReturn, textBuffer);
      while (bufIndex > 0) {
        bufIndex--;
        textBuffer[bufIndex] = 0;
      }
      // clear writing pad
      tft.fillRect(15, kTextAreaHeight - 30, tft.width() - 40, 20, kDisplayBackroundColor);

      DrawKeyboardButton(KB_ENTER_KEY, /*bool clicked*/ true);
      delay(kUserInputDelayMs);
      DrawKeyboardButton(KB_ENTER_KEY, /*bool clicked*/ false);
    }
  }

  // display current textBuffer
  tft.setTextColor(kTextRegularColor, kKeyboardButtonFillColor);
  tft.setCursor(15, kTextAreaHeight - 30);
  tft.print(textBuffer);
  // Serial.println(textBuffer);
  return true;
}

// get user text input from on-screen keyboard
bool RGBDisplay::GetUserOnScreenTextInput(std::string label, char* return_text, bool numbers_only, bool alphabets_only) {
  bool ret = false;

  tft.fillScreen(kDisplayBackroundColor);
  tft.setFont(NULL);

  kb_numbers_only = numbers_only;
  kb_alphabets_only = alphabets_only;
  if(kb_numbers_only) {
    // Numpad Input
    GetKeyboardPress_shift = false;
    GetKeyboardPress_lastShift = false;
    GetKeyboardPress_numpad = true;
    GetKeyboardPress_lastNumpad = true;
    MakeKeyboard(Mobile_NumKeys, label);
  }
  else if(kb_alphabets_only) {
    // Alphabets Input
    GetKeyboardPress_shift = true;
    GetKeyboardPress_lastShift = true;
    GetKeyboardPress_numpad = false;
    GetKeyboardPress_lastNumpad = false;
    MakeKeyboard(Mobile_KB_Capitals, label);
  }
  else {
    // Keypad Input
    GetKeyboardPress_shift = false;
    GetKeyboardPress_lastShift = false;
    GetKeyboardPress_numpad = false;
    GetKeyboardPress_lastNumpad = false;
    MakeKeyboard(Mobile_KB_Smalls, label);
  }

  // buffer for user input
  char user_input_buffer[kWifiSsidPasswordLengthMax + 1] = "";
  strcpy(user_input_buffer, return_text);
  strcpy(return_text, "");

  // get user input
  while(1) {
    ResetWatchdog();
    inactivity_millis = 0;

    // See if there's any  touch data for us
    ret = GetKeyboardPress(user_input_buffer, label, return_text);

    // break if back button is pressed
    if(!ret)
      break;

    //print the text
    tft.setCursor(10,30);
    tft.println(return_text);
    if(strcmp(return_text, "") != 0) {
      PrintLn(return_text);
      delay(1000);
      break;
    }
  }
  // change back text size to default
  tft.setTextSize(1);
  return ret;
}

void RGBDisplay::TouchCalibrationScreen(int16_t x0, int16_t y0, int16_t x1, int16_t y1, bool touched, bool redraw) {
  if(redraw) {
    tft.fillScreen(kDisplayBackroundColor);
    
    std::string title_str = "Calibrate Touchscreen";
    std::string direction_str = "\"Tap\" the shown line using\n a blunt 1-1.5mm dia stylus";

    // set font
    tft.setFont(&FreeSansBold12pt7b);
    // get bounds of text on screen
    tft.setTextColor(kDisplayBackroundColor);
    uint16_t title_str_h = 0, title_str_w = 0;
    int16_t title_str_gap_x = 0, title_str_gap_y = 0;
    tft.getTextBounds(title_str.c_str(), 0, 0, &title_str_gap_x, &title_str_gap_y, &title_str_w, &title_str_h);

    // set font
    tft.setFont(&FreeMono9pt7b);
    // get bounds of text on screen
    uint16_t direction_str_h = 0, direction_str_w = 0;
    int16_t direction_str_gap_x = 0, direction_str_gap_y = 0;
    tft.getTextBounds(direction_str.c_str(), 0, 0, &direction_str_gap_x, &direction_str_gap_y, &direction_str_w, &direction_str_h);

    // change the text color to the background color
    tft.setTextColor(kDisplayColorGreen);
    tft.setFont(&FreeSansBold12pt7b);
    // yes! home the cursor
    tft.setCursor((kTftWidth - title_str_w) / 2, kTftHeight / 2 - title_str_h);
    tft.print(title_str.c_str());

    tft.setTextColor(kDisplayColorYellow);
    tft.setFont(&FreeMono9pt7b);
    // yes! home the cursor
    tft.setCursor((kTftWidth - direction_str_w) / 2, kTftHeight / 2 + direction_str_h / 2);
    tft.print(direction_str.c_str());
  }

  // draw target line
  tft.drawLine(x0, y0, x1, y1, kDisplayColorOrange);

  // show touch feedback
  if(touched)
    tft.drawLine(x0, y0, x1, y1, kDisplayColorCyan);
}

void RGBDisplay::TouchCalibrationScreenTest(int16_t x_target, int16_t y_target, int16_t x_touch, int16_t y_touch, bool redraw) {
  if(redraw) {
    tft.fillScreen(kDisplayBackroundColor);
    
    std::string title_str = "Test Touchscreen";

    // set font
    tft.setFont(&FreeSansBold12pt7b);
    // get bounds of text on screen
    tft.setTextColor(kDisplayBackroundColor);
    uint16_t title_str_h = 0, title_str_w = 0;
    int16_t title_str_gap_x = 0, title_str_gap_y = 0;
    tft.getTextBounds(title_str.c_str(), 0, 0, &title_str_gap_x, &title_str_gap_y, &title_str_w, &title_str_h);
    // std::string log_str = "title_str: " + title_str + "  gap_x=" + std::to_string(title_str_gap_x) + ", gap_y=" + std::to_string(title_str_gap_y) + ", w=" + std::to_string(title_str_w) + ", h=" + std::to_string(title_str_h);
    // PrintLn(log_str);

    // change the text color to the background color
    tft.setTextColor(kDisplayColorGreen);
    tft.setFont(&FreeSansBold12pt7b);
    // yes! home the cursor
    tft.setCursor((kTftWidth - title_str_w) / 2, kTftHeight / 2 - title_str_h / 2);
    tft.print(title_str.c_str());

    title_str = "Calibration";
    // get bounds of text on screen
    tft.setTextColor(kDisplayBackroundColor);
    title_str_h = 0, title_str_w = 0;
    title_str_gap_x = 0, title_str_gap_y = 0;
    tft.getTextBounds(title_str.c_str(), 0, 0, &title_str_gap_x, &title_str_gap_y, &title_str_w, &title_str_h);
    // log_str = "title_str: " + title_str + "  gap_x=" + std::to_string(title_str_gap_x) + ", gap_y=" + std::to_string(title_str_gap_y) + ", w=" + std::to_string(title_str_w) + ", h=" + std::to_string(title_str_h);
    // PrintLn(log_str);

    // change the text color to the background color
    tft.setTextColor(kDisplayColorGreen);
    tft.setFont(&FreeSansBold12pt7b);
    // yes! home the cursor
    tft.setCursor((kTftWidth - title_str_w) / 2, kTftHeight / 2 + 3 * title_str_h / 2);
    tft.print(title_str.c_str());
  }

  // draw target:
  tft.fillCircle(x_target, y_target, 2, kDisplayColorOrange);
  tft.drawCircle(x_target, y_target, 6, kDisplayColorOrange);
  // draw touch
  tft.fillCircle(x_touch, y_touch, 2, kDisplayColorCyan);

}
#ifndef PIN_DEFS_H
#define PIN_DEFS_H

#include "configuration.h"

// define pins
#if defined(MCU_IS_ESP32_S3)

  // FOR ESP32 S3 DECKIT C1 MODULE
  // dual core

  #define MCU_IS_ESP32
  #define ESP32_DUAL_CORE

  const uint8_t TFT_COPI = 11;
  const uint8_t TFT_CLK = 12;
  const uint8_t TFT_CS = 10;
  const uint8_t TFT_RST = 14;  // Or set to -1 and connect to Arduino RESET pin
  const uint8_t TFT_DC = 21;
  const uint8_t TFT_BL = 47;  //  controls TFT Display backlight as output of PWM pin

  const uint8_t TS_CIPO = 13;    // don't connect CIPO (MISO) to TFT
  const uint8_t TS_CS_PIN = 17;
  const uint8_t TS_IRQ_PIN = 48;

  // Sqw Alarm Interrupt Pin
  const uint8_t SQW_INT_PIN = 18;
  const uint8_t SDA_PIN = 8;
  const uint8_t SCL_PIN = 9;
  const uint8_t BUTTON_PIN = 1;
  const uint8_t INC_BUTTON_PIN = 2;
  const uint8_t DEC_BUTTON_PIN = 42;
  // #define BUTTON_PIN_BITMASK 0x800000000  // 2^35 in hex
  const uint8_t LED_PIN = 41;
  const uint8_t WIFI_LED = 39;
  const uint8_t BUZZER_PIN = 16;
  const uint8_t DEBUG_PIN = 40;    // manually pull down to enable debug mode, watchdog reboot will not be used in debug mode
  const uint8_t PHOTORESISTOR_PIN = 4;    // ADC1 GPIO
  const uint8_t RGB_LED_STRIP_PIN = 15;
  const uint8_t TOUCH_PIN_5 = 5;
  const uint8_t TOUCHSCREEN_XP = 19;
  const uint8_t TOUCHSCREEN_XM = 7;   // ADC1 GPIO
  const uint8_t TOUCHSCREEN_YP = 3;   // ADC1 GPIO
  const uint8_t TOUCHSCREEN_YM = 20;


#elif defined(MCU_IS_ESP32_S2_MINI)

  // FOR ESP32 S2 MINI MODULE
  // single core

  #define MCU_IS_ESP32
  #define ESP32_SINGLE_CORE

  const int TFT_COPI = 35;
  const int TFT_CLK = 36;
  const int TFT_CS = 34;
  const int TFT_RST = 33;  // Or set to -1 and connect to Arduino RESET pin
  const int TFT_DC = 38;
  const int TFT_BL = 17;  //  controls TFT Display backlight as output of PWM pin

  const int TS_CIPO = 37;    // don't connect CIPO (MISO) to TFT
  const int TS_CS_PIN = 2;
  const int TS_IRQ_PIN = 3;

  // Sqw Alarm Interrupt Pin
  const int SQW_INT_PIN = 7;
  const int SDA_PIN = 8;
  const int SCL_PIN = 9;
  const int BUTTON_PIN = 6;
  const int INC_BUTTON_PIN = 10;
  const int DEC_BUTTON_PIN = 11;
  // #define BUTTON_PIN_BITMASK 0x800000000  // 2^35 in hex
  const int LED_PIN = 5;
  // const int LED_BUILTIN = 15;   // pre-defined
  const int WIFI_LED = 15;
  const int BUZZER_PIN = 4;
  const int DEBUG_PIN = 21;    // manually pull down to enable debug mode, watchdog reboot will not be used in debug
  const int PHOTORESISTOR_PIN = 1;
  const int RGB_LED_STRIP_PIN = 14;



#elif defined(MCU_IS_ESP32_WROOM_DA_MODULE)

  // FOR ESP32 WROOM DA MODULE
  // dual core

  #define MCU_IS_ESP32
  #define ESP32_DUAL_CORE

  const int TFT_COPI = 23;
  const int TFT_CLK = 18;
  const int TFT_CS = 16;
  const int TFT_RST = 27;  // Or set to -1 and connect to Arduino RESET pin
  const int TFT_DC = 26;
  const int TFT_BL = 14;  //  controls TFT Display backlight as output of PWM pin

  const int TS_CIPO = 19;    // don't connect CIPO (MISO) to TFT
  const int TS_CS_PIN = 33;
  const int TS_IRQ_PIN = 34;

  // Sqw Alarm Interrupt Pin
  const int SQW_INT_PIN = 4;
  const int SDA_PIN = 21;
  const int SCL_PIN = 22;
  const int BUTTON_PIN = 35;
  #define BUTTON_PIN_BITMASK 0x800000000  // 2^35 in hex
  const int INC_BUTTON_PIN = 34;
  const int DEC_BUTTON_PIN = 33;
  const int LED_PIN = 32;
  // const int LED_BUILTIN = 2;
  const int WIFI_LED = 2;
  const int BUZZER_PIN = 13;
  const int DEBUG_PIN = 12;    // manually pull down to enable debug mode, watchdog reboot will not be used in debug mode
  const int PHOTORESISTOR_PIN = 25;
  const int RGB_LED_STRIP_PIN = 5;


#endif


#endif  // PIN_DEFS_H
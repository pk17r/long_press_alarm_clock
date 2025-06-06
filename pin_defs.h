#ifndef PIN_DEFS_H
#define PIN_DEFS_H

#include "configuration.h"

extern uint8_t My_Hw_Version;

// define pins
#if defined(MCU_IS_ESP32_S3)

  // FOR ESP32 S3 MODULE, with/out OPSI PSRAM, 4Mb flash gets used up around 90% with ESP_ARDUINO_VERSION 2.x.x
  // dual core

  #define MCU_IS_ESP32
  #define ESP32_DUAL_CORE

  const uint8_t TFT_COPI = 11;
  const uint8_t TFT_CLK = 12;
  const uint8_t TFT_CS = 10;
  const uint8_t TFT_RST = 14;  // Or set to -1 and connect to Arduino RESET pin
  const uint8_t TFT_DC = 21;
  //  controls TFT Display backlight as output of PWM pin. analogWrite works on PWM not ADC.
  const uint8_t TFT_BL = 47;

  const uint8_t TS_CIPO = 13;    // don't connect CIPO (MISO) to Display
  const uint8_t TS_CS_PIN = 17;
  const uint8_t TS_IRQ_PIN = 48;

  // Sqw Alarm Interrupt Pin
  static uint8_t SDA_PIN() { return (My_Hw_Version == 0x01 ? 8 : (My_Hw_Version == 0x02 ? 16 : 0xff)); }
  static uint8_t SCL_PIN() { return (My_Hw_Version == 0x01 ? 9 : (My_Hw_Version == 0x02 ? 17 : 0xff)); }
  const uint8_t SQW_INT_PIN = 18;
  static uint8_t BUTTON_PIN() { return (My_Hw_Version == 0x01 ? 1 : (My_Hw_Version == 0x02 ? 8 : 0xff)); }
  const uint8_t INC_BUTTON_PIN = 2;
  const uint8_t DEC_BUTTON_PIN = 42;
  // #define BUTTON_PIN_BITMASK 0x800000000  // 2^35 in hex
  static uint8_t LED_PIN() { return (My_Hw_Version == 0x01 ? 41 : (My_Hw_Version == 0x02 ? 6 : 0xff)); }
  static uint8_t WIFI_LED() { return (My_Hw_Version == 0x01 ? 39 : 0xff); }
  static uint8_t BUZZER_PIN() { return (My_Hw_Version == 0x01 ? 16 : (My_Hw_Version == 0x02 ? 38 : 0xff)); }
  const uint8_t DEBUG_PIN = 40;    // manually pull down to enable debug mode, watchdog reboot will not be used in debug mode
  static uint8_t PHOTORESISTOR_PIN() { return (My_Hw_Version == 0x01 ? 4 : (My_Hw_Version == 0x02 ? 7 : 0xff)); }    // ADC1 GPIO
  static uint8_t RGB_LED_STRIP_PIN() { return (My_Hw_Version == 0x01 ? 15 : (My_Hw_Version == 0x02 ? 5 : 0xff)); }
  // const uint8_t TOUCH_PIN_5 = 5;
  const uint8_t TOUCHSCREEN_XP = 19;
  static uint8_t TOUCHSCREEN_XM_ADC() { return (My_Hw_Version == 0x01 ? 7 : (My_Hw_Version == 0x02 ? 9 : 0xff)); }     // ADC1 GPIO
  const uint8_t TOUCHSCREEN_YP_ADC = 3;   // ADC1 GPIO
  const uint8_t TOUCHSCREEN_YM = 20;


#elif defined(MCU_IS_ESP32_S2_MINI)

  // FOR ESP32 S2 MINI MODULE, with/out PSRAM, 4Mb flash gets used upto around 87% with ESP_ARDUINO_VERSION 2.x.x
  // single core

  #define MCU_IS_ESP32
  #define ESP32_SINGLE_CORE

  const uint8_t TFT_COPI = 35;
  const uint8_t TFT_CLK = 36;
  const uint8_t TFT_CS = 34;
  const uint8_t TFT_RST = 33;  // Or set to -1 and connect to Arduino RESET pin
  const uint8_t TFT_DC = 38;
  //  controls TFT Display backlight as output of PWM pin. analogWrite works on PWM not ADC.
  const uint8_t TFT_BL = 17;

  const uint8_t TS_CIPO = 37;    // don't connect CIPO (MISO) to Display
  const uint8_t TS_CS_PIN = 2;
  const uint8_t TS_IRQ_PIN = 3;

  // Sqw Alarm Interrupt Pin
  static uint8_t SDA_PIN() { return (My_Hw_Version == 0x01 ? 8 : (My_Hw_Version == 0x02 ? 5 : 0xff)); }
  static uint8_t SCL_PIN() { return (My_Hw_Version == 0x01 ? 9 : (My_Hw_Version == 0x02 ? 6 : 0xff)); }
  const uint8_t SQW_INT_PIN = 7;
  static uint8_t BUTTON_PIN() { return (My_Hw_Version == 0x01 ? 6 : (My_Hw_Version == 0x02 ? 8 : 0xff)); }
  const uint8_t INC_BUTTON_PIN = 10;
  const uint8_t DEC_BUTTON_PIN = 11;
  // #define BUTTON_PIN_BITMASK 0x800000000  // 2^35 in hex
  static uint8_t LED_PIN() { return (My_Hw_Version == 0x01 ? 5 : (My_Hw_Version == 0x02 ? 3 : 0xff)); }
  // const uint8_t LED_BUILTIN = 15;   // pre-defined
  static uint8_t WIFI_LED() { return 15; }
  static const uint8_t BUZZER_PIN() { return (My_Hw_Version == 0x01 ? 4 : (My_Hw_Version == 0x02 ? 40 : 0xff)); }
  const uint8_t DEBUG_PIN = 21;    // manually pull down to enable debug mode, watchdog reboot will not be used in debug
  static const uint8_t PHOTORESISTOR_PIN() { return (My_Hw_Version == 0x01 ? 1 : (My_Hw_Version == 0x02 ? 4 : 0xff)); }
  static uint8_t RGB_LED_STRIP_PIN() { return (My_Hw_Version == 0x01 ? 14 : (My_Hw_Version == 0x02 ? 2 : 0xff)); }
  const uint8_t TOUCHSCREEN_XP = 12;
  static uint8_t TOUCHSCREEN_XM_ADC() { return 10; }     // ADC1 GPIO
  const uint8_t TOUCHSCREEN_YP_ADC = 9;   // ADC1 GPIO
  const uint8_t TOUCHSCREEN_YM = 13;


#elif defined(MCU_IS_ESP32_WROOM_DA_MODULE)

  // FOR ESP32 WROOM DA MODULE
  // dual core

  #define MCU_IS_ESP32
  #define ESP32_DUAL_CORE

  const uint8_t TFT_COPI = 23;
  const uint8_t TFT_CLK = 18;
  const uint8_t TFT_CS = 16;
  const uint8_t TFT_RST = 27;  // Or set to -1 and connect to Arduino RESET pin
  const uint8_t TFT_DC = 26;
  //  controls TFT Display backlight as output of PWM pin. analogWrite works on PWM not ADC.
  const uint8_t TFT_BL = 14;

  const uint8_t TS_CIPO = 19;    // don't connect CIPO (MISO) to Display
  const uint8_t TS_CS_PIN = 33;
  const uint8_t TS_IRQ_PIN = 34;

  // Sqw Alarm Interrupt Pin
  static uint8_t SDA_PIN() { return 21; }
  static uint8_t SCL_PIN() { return 22; }
  const uint8_t SQW_INT_PIN = 4;
  static uint8_t BUTTON_PIN() { return 35; }
  #define BUTTON_PIN_BITMASK 0x800000000  // 2^35 in hex
  const uint8_t INC_BUTTON_PIN = 34;
  const uint8_t DEC_BUTTON_PIN = 33;
  static uint8_t LED_PIN() { return 32; }
  // const uint8_t LED_BUILTIN = 2;
  static uint8_t WIFI_LED() { return 2; }
  static const uint8_t BUZZER_PIN() { return 13; }
  const uint8_t DEBUG_PIN = 12;    // manually pull down to enable debug mode, watchdog reboot will not be used in debug mode
  static const uint8_t PHOTORESISTOR_PIN() { return 25; }
  static uint8_t RGB_LED_STRIP_PIN() { return 5; }


#endif


#endif  // PIN_DEFS_H
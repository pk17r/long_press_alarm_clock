#ifndef PIN_DEFS_H
#define PIN_DEFS_H

#include "configuration.h"

extern uint8_t My_Hw_Version;

// define pins
#if defined(MCU_IS_ESP32_S2)

  // My_Hw_Version = 0x02

  // FOR ESP32 S2 with PSRAM MODULE, 4Mb flash gets used upto around 93% with ESP_ARDUINO_VERSION 2.x.x
  // single core

  #define MCU_IS_ESP32
  #define ESP32_SINGLE_CORE

  const uint8_t SPI_MOSI = 35;
  const uint8_t SPI_CLK = 36;
  const uint8_t SPI_MISO = 37;    // don't connect MISO to Display

  const uint8_t DISPLAY_CS = 34;
  const uint8_t DISPLAY_RES = 33;  // Or set to -1 and connect to Arduino RESET pin
  const uint8_t DISPLAY_DC = 38;
  //  controls TFT Display backlight as output of PWM pin. analogWrite works on PWM not ADC.
  const uint8_t DISPLAY_BL = 17;

  const uint8_t TS_CS = 2;
  const uint8_t TS_IRQ = 3;

  // Sqw Alarm Interrupt Pin
  static uint8_t SDA_PIN() { return 4; }
  static uint8_t SCL_PIN() { return 5; }
  const uint8_t SQW_INT_PIN = 7;
  static uint8_t BUTTON_PIN() { return 8; }
  static uint8_t INC_BUTTON_PIN() { return 0xff; }
  static uint8_t DEC_BUTTON_PIN() { return 0xff; }
  // #define BUTTON_PIN_BITMASK 0x800000000  // 2^35 in hex
  static uint8_t LED_PIN() { return 2; }
  // const uint8_t LED_BUILTIN = 15;   // pre-defined
  static uint8_t WIFI_LED() { return 15; }
  static uint8_t BUZZER_PIN() { return 40; }
  const uint8_t DEBUG_PIN = 21;    // manually pull down to enable debug mode, watchdog reboot will not be used in debug
  static uint8_t PHOTORESISTOR_PIN() { return 3; }
  static uint8_t RGB_LED_STRIP_PIN() { return 1; }
  const uint8_t TOUCHSCREEN_XP = 12;
  static uint8_t TOUCHSCREEN_XM_ADC() { return 10; }     // ADC1 GPIO
  const uint8_t TOUCHSCREEN_YP_ADC = 9;   // ADC1 GPIO
  const uint8_t TOUCHSCREEN_YM = 13;
  static uint8_t POWER_RAIL_5V_CONTROL() { return 39; }


#elif defined(MCU_IS_ESP32_S3)

  // My_Hw_Version = 0x01

  // FOR ESP32 S3 MODULE, with/out OPSI PSRAM, 4Mb flash gets used up around 90% with ESP_ARDUINO_VERSION 2.x.x
  // dual core

  #define MCU_IS_ESP32
  #define ESP32_DUAL_CORE

  const uint8_t SPI_MOSI = 11;
  const uint8_t SPI_CLK = 12;
  const uint8_t DISPLAY_CS = 10;
  const uint8_t DISPLAY_RES = 14;  // Or set to -1 and connect to Arduino RESET pin
  const uint8_t DISPLAY_DC = 21;
  //  controls TFT Display backlight as output of PWM pin. analogWrite works on PWM not ADC.
  const uint8_t DISPLAY_BL = 47;

  const uint8_t SPI_MISO = 13;    // don't connect CIPO (MISO) to Display
  const uint8_t TS_CS = 17;
  const uint8_t TS_IRQ = 48;

  // Sqw Alarm Interrupt Pin
  static uint8_t SDA_PIN() { return 8; }
  static uint8_t SCL_PIN() { return 9; }
  const uint8_t SQW_INT_PIN = 18;
  static uint8_t BUTTON_PIN() { return 1; }
  static uint8_t INC_BUTTON_PIN() { return 2; }
  static uint8_t DEC_BUTTON_PIN() { return 42; }
  // #define BUTTON_PIN_BITMASK 0x800000000  // 2^35 in hex
  static uint8_t LED_PIN() { return 41; }
  static uint8_t WIFI_LED() { return 39; }
  static uint8_t BUZZER_PIN() { return 16; }
  const uint8_t DEBUG_PIN = 40;    // manually pull down to enable debug mode, watchdog reboot will not be used in debug mode
  static uint8_t PHOTORESISTOR_PIN() { return 4; }    // ADC1 GPIO
  static uint8_t RGB_LED_STRIP_PIN() { return 15; }
  // const uint8_t TOUCH_PIN_5 = 5;
  const uint8_t TOUCHSCREEN_XP = 19;
  static uint8_t TOUCHSCREEN_XM_ADC() { return 7; }     // ADC1 GPIO
  const uint8_t TOUCHSCREEN_YP_ADC = 3;   // ADC1 GPIO
  const uint8_t TOUCHSCREEN_YM = 20;
  static uint8_t POWER_RAIL_5V_CONTROL() { return 0xff; }

  // HW2 Pinout chosen on Easy EDA - for future reference
  // // Sqw Alarm Interrupt Pin
  // static uint8_t SDA_PIN() { return 16; }
  // static uint8_t SCL_PIN() { return 17; }
  // const uint8_t SQW_INT_PIN = 18;
  // static uint8_t BUTTON_PIN() { return 8; }
  // static uint8_t INC_BUTTON_PIN() { return 0xff; }
  // static uint8_t DEC_BUTTON_PIN() { return 0xff; }
  // // #define BUTTON_PIN_BITMASK 0x800000000  // 2^35 in hex
  // static uint8_t LED_PIN() { return 6; }
  // static uint8_t WIFI_LED() { return 0xff; }
  // static uint8_t BUZZER_PIN() { return 38; }
  // const uint8_t DEBUG_PIN = 40;    // manually pull down to enable debug mode, watchdog reboot will not be used in debug mode
  // static uint8_t PHOTORESISTOR_PIN() { return 7; }    // ADC1 GPIO
  // static uint8_t RGB_LED_STRIP_PIN() { return 5; }
  // // const uint8_t TOUCH_PIN_5 = 5;
  // const uint8_t TOUCHSCREEN_XP = 19;
  // static uint8_t TOUCHSCREEN_XM_ADC() { return 9; }     // ADC1 GPIO
  // const uint8_t TOUCHSCREEN_YP_ADC = 3;   // ADC1 GPIO
  // const uint8_t TOUCHSCREEN_YM = 20;
  // static uint8_t POWER_RAIL_5V_CONTROL() { return 0xff; }


#elif defined(MCU_IS_ESP32_S2_MINI)

  // My_Hw_Version = 0x01

  // FOR ESP32 S2 MINI MODULE, with/out PSRAM, 4Mb flash gets used upto around 87% with ESP_ARDUINO_VERSION 2.x.x
  // single core

  #define MCU_IS_ESP32
  #define ESP32_SINGLE_CORE

  const uint8_t SPI_MOSI = 35;
  const uint8_t SPI_CLK = 36;
  const uint8_t SPI_MISO = 37;    // don't connect MISO to Display

  const uint8_t DISPLAY_CS = 34;
  const uint8_t DISPLAY_RES = 33;  // Or set to -1 and connect to Arduino RESET pin
  const uint8_t DISPLAY_DC = 38;
  //  controls TFT Display backlight as output of PWM pin. analogWrite works on PWM not ADC.
  const uint8_t DISPLAY_BL = 17;

  const uint8_t TS_CS = 2;
  const uint8_t TS_IRQ = 3;

  // Sqw Alarm Interrupt Pin
  static uint8_t SDA_PIN() { return 8; }
  static uint8_t SCL_PIN() { return 9; }
  const uint8_t SQW_INT_PIN = 7;
  static uint8_t BUTTON_PIN() { return 6; }
  static uint8_t INC_BUTTON_PIN() { return 10; }
  static uint8_t DEC_BUTTON_PIN() { return 11; }
  // #define BUTTON_PIN_BITMASK 0x800000000  // 2^35 in hex
  static uint8_t LED_PIN() { return 5; }
  // const uint8_t LED_BUILTIN = 15;   // pre-defined
  static uint8_t WIFI_LED() { return 15; }
  static uint8_t BUZZER_PIN() { return 4; }
  const uint8_t DEBUG_PIN = 21;    // manually pull down to enable debug mode, watchdog reboot will not be used in debug
  static uint8_t PHOTORESISTOR_PIN() { return 1; }
  static uint8_t RGB_LED_STRIP_PIN() { return 14; }
  const uint8_t TOUCHSCREEN_XP = 12;
  static uint8_t TOUCHSCREEN_XM_ADC() { return 10; }     // ADC1 GPIO
  const uint8_t TOUCHSCREEN_YP_ADC = 9;   // ADC1 GPIO
  const uint8_t TOUCHSCREEN_YM = 13;
  static uint8_t POWER_RAIL_5V_CONTROL() { return 0xff; }


#elif defined(MCU_IS_ESP32_WROOM_DA_MODULE)

  // My_Hw_Version = 0x01

  // FOR ESP32 WROOM DA MODULE
  // dual core

  #define MCU_IS_ESP32
  #define ESP32_DUAL_CORE

  const uint8_t SPI_MOSI = 23;
  const uint8_t SPI_CLK = 18;
  const uint8_t DISPLAY_CS = 16;
  const uint8_t DISPLAY_RES = 27;  // Or set to -1 and connect to Arduino RESET pin
  const uint8_t DISPLAY_DC = 26;
  //  controls TFT Display backlight as output of PWM pin. analogWrite works on PWM not ADC.
  const uint8_t DISPLAY_BL = 14;

  const uint8_t SPI_MISO = 19;    // don't connect CIPO (MISO) to Display
  const uint8_t TS_CS = 33;
  const uint8_t TS_IRQ = 34;

  // Sqw Alarm Interrupt Pin
  static uint8_t SDA_PIN() { return 21; }
  static uint8_t SCL_PIN() { return 22; }
  const uint8_t SQW_INT_PIN = 4;
  static uint8_t BUTTON_PIN() { return 35; }
  #define BUTTON_PIN_BITMASK 0x800000000  // 2^35 in hex
  static uint8_t INC_BUTTON_PIN() { return 34; }
  static uint8_t DEC_BUTTON_PIN() { return 33; }
  static uint8_t LED_PIN() { return 32; }
  // const uint8_t LED_BUILTIN = 2;
  static uint8_t WIFI_LED() { return 2; }
  static uint8_t BUZZER_PIN() { return 13; }
  const uint8_t DEBUG_PIN = 12;    // manually pull down to enable debug mode, watchdog reboot will not be used in debug mode
  static uint8_t PHOTORESISTOR_PIN() { return 25; }
  static uint8_t RGB_LED_STRIP_PIN() { return 5; }
  const uint8_t TOUCHSCREEN_XP = 16;
  static uint8_t TOUCHSCREEN_XM_ADC() { return 39; }     // ADC1 GPIO
  const uint8_t TOUCHSCREEN_YP_ADC = 36;   // ADC1 GPIO
  const uint8_t TOUCHSCREEN_YM = 4;
  static uint8_t POWER_RAIL_5V_CONTROL() { return 0xff; }

#endif


#endif  // PIN_DEFS_H
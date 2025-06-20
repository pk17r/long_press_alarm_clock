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
  const uint8_t SDA_PIN = 4;
  const uint8_t SCL_PIN = 5;
  const uint8_t SQW_INT_PIN = 7;
  const uint8_t BUTTON_PIN = 8;
  const uint8_t INC_BUTTON_PIN = 0xff;
  const uint8_t DEC_BUTTON_PIN = 0xff;
  // #define BUTTON_PIN_BITMASK 0x800000000  // 2^35 in hex
  const uint8_t LED_PIN = 2;
  // const uint8_t LED_BUILTIN = 15;   // pre-defined
  const uint8_t WIFI_LED = 15;
  const uint8_t BUZZER_PIN = 40;
  const uint8_t DEBUG_PIN = 21;    // manually pull down to enable debug mode, watchdog reboot will not be used in debug
  const uint8_t PHOTORESISTOR_PIN = 3;
  const uint8_t RGB_LED_STRIP_PIN = 1;
  const uint8_t TOUCHSCREEN_XP = 12;
  const uint8_t TOUCHSCREEN_XM_ADC = 10;     // ADC1 GPIO
  const uint8_t TOUCHSCREEN_YP_ADC = 9;   // ADC1 GPIO
  const uint8_t TOUCHSCREEN_YM = 13;
  const uint8_t POWER_RAIL_5V_CONTROL = 39;


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
  const uint8_t SDA_PIN = 8;
  const uint8_t SCL_PIN = 9;
  const uint8_t SQW_INT_PIN = 18;
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
  // const uint8_t TOUCH_PIN_5 = 5;
  const uint8_t TOUCHSCREEN_XP = 19;
  const uint8_t TOUCHSCREEN_XM_ADC = 7;     // ADC1 GPIO
  const uint8_t TOUCHSCREEN_YP_ADC = 3;   // ADC1 GPIO
  const uint8_t TOUCHSCREEN_YM = 20;
  const uint8_t POWER_RAIL_5V_CONTROL = 0xff;

  // HW2 Pinout chosen on Easy EDA - for future reference
  // // Sqw Alarm Interrupt Pin
  // const uint8_t SDA_PIN = 16;
  // const uint8_t SCL_PIN = 17;
  // const uint8_t SQW_INT_PIN = 18;
  // const uint8_t BUTTON_PIN = 8;
  // const uint8_t INC_BUTTON_PIN = 0xff;
  // const uint8_t DEC_BUTTON_PIN = 0xff;
  // // #define BUTTON_PIN_BITMASK 0x800000000  // 2^35 in hex
  // const uint8_t LED_PIN = 6;
  // const uint8_t WIFI_LED = 0xff;
  // const uint8_t BUZZER_PIN = 38;
  // const uint8_t DEBUG_PIN = 40;    // manually pull down to enable debug mode, watchdog reboot will not be used in debug mode
  // const uint8_t PHOTORESISTOR_PIN = 7;    // ADC1 GPIO
  // const uint8_t RGB_LED_STRIP_PIN = 5;
  // // const uint8_t TOUCH_PIN_5 = 5;
  // const uint8_t TOUCHSCREEN_XP = 19;
  // const uint8_t TOUCHSCREEN_XM_ADC = 9;     // ADC1 GPIO
  // const uint8_t TOUCHSCREEN_YP_ADC = 3;   // ADC1 GPIO
  // const uint8_t TOUCHSCREEN_YM = 20;
  // const uint8_t POWER_RAIL_5V_CONTROL = 0xff;


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
  const uint8_t SDA_PIN = 8;
  const uint8_t SCL_PIN = 9;
  const uint8_t SQW_INT_PIN = 7;
  const uint8_t BUTTON_PIN = 6;
  const uint8_t INC_BUTTON_PIN = 10;
  const uint8_t DEC_BUTTON_PIN = 11;
  // #define BUTTON_PIN_BITMASK 0x800000000  // 2^35 in hex
  const uint8_t LED_PIN = 5;
  // const uint8_t LED_BUILTIN = 15;   // pre-defined
  const uint8_t WIFI_LED = 15;
  const uint8_t BUZZER_PIN = 4;
  const uint8_t DEBUG_PIN = 21;    // manually pull down to enable debug mode, watchdog reboot will not be used in debug
  const uint8_t PHOTORESISTOR_PIN = 1;
  const uint8_t RGB_LED_STRIP_PIN = 14;
  const uint8_t TOUCHSCREEN_XP = 12;
  const uint8_t TOUCHSCREEN_XM_ADC = 10;     // ADC1 GPIO
  const uint8_t TOUCHSCREEN_YP_ADC = 9;   // ADC1 GPIO
  const uint8_t TOUCHSCREEN_YM = 13;
  const uint8_t POWER_RAIL_5V_CONTROL = 0xff;


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
  const uint8_t SDA_PIN = 21;
  const uint8_t SCL_PIN = 22;
  const uint8_t SQW_INT_PIN = 4;
  const uint8_t BUTTON_PIN = 35;
  #define BUTTON_PIN_BITMASK 0x800000000  // 2^35 in hex
  const uint8_t INC_BUTTON_PIN = 34;
  const uint8_t DEC_BUTTON_PIN = 33;
  const uint8_t LED_PIN = 32;
  // const uint8_t LED_BUILTIN = 2;
  const uint8_t WIFI_LED = 2;
  const uint8_t BUZZER_PIN = 13;
  const uint8_t DEBUG_PIN = 12;    // manually pull down to enable debug mode, watchdog reboot will not be used in debug mode
  const uint8_t PHOTORESISTOR_PIN = 25;
  const uint8_t RGB_LED_STRIP_PIN = 5;
  const uint8_t TOUCHSCREEN_XP = 16;
  const uint8_t TOUCHSCREEN_XM_ADC = 39;     // ADC1 GPIO
  const uint8_t TOUCHSCREEN_YP_ADC = 36;   // ADC1 GPIO
  const uint8_t TOUCHSCREEN_YM = 4;
  const uint8_t POWER_RAIL_5V_CONTROL = 0xff;

#endif


#endif  // PIN_DEFS_H
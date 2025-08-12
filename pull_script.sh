#!/bin/bash
echo "Hello, World!"
cp ../long_press_alarm_clock_dev/build/esp32.esp32.lolin_s2_mini/long_press_alarm_clock_dev.ino.bin build/esp32.esp32.lolin_s2_mini/long_press_alarm_clock.ino.bin
cp ../long_press_alarm_clock_dev/build/esp32.esp32.esp32s3/long_press_alarm_clock_dev.ino.bin build/esp32.esp32.esp32s3/long_press_alarm_clock.ino.bin
cp ../long_press_alarm_clock_dev/build/esp32.esp32.esp32s2/long_press_alarm_clock_dev.ino.bin build/esp32.esp32.esp32s2/long_press_alarm_clock.ino.bin
cp ../long_press_alarm_clock_dev/build/esp32.esp32.esp32da/long_press_alarm_clock_dev.ino.bin build/esp32.esp32.esp32da/long_press_alarm_clock.ino.bin
cp ../long_press_alarm_clock_dev/configuration.h .
echo "Copy Done!"


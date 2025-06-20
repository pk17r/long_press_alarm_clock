#ifndef ALARM_CLOCK_H
#define ALARM_CLOCK_H

#include "common.h"
// include files for timer
#include <stdio.h>

class AlarmClock {

public:

// FUNCTIONS

  // function declerations
  void Setup();
  void SaveAlarm();
  int16_t MinutesToAlarm();
  void BuzzAlarmFn();
  void playNote(int frequency, int duration, bool hold = false);
  void celebrateSong(int &tone_note_index, unsigned long &next_tone_change_time);


// OBJECTS and VARIABLES

  // alarm time
  uint8_t alarm_hr_ = 7;
  uint8_t alarm_min_ = 0;
  bool alarm_is_AM_ = true;
  bool alarm_ON_ = true;    // flag to set alarm On or Off

  // Alarm variables & constants
  uint8_t alarm_long_press_seconds_ = 25;
  const unsigned long kAlarmMaxON_TimeMs = 120*1000;

  // Set Screen variables
  uint8_t var_1_ = alarm_hr_;
  uint8_t var_2_ = alarm_min_;
  bool var_3_is_AM_ = alarm_is_AM_;
  bool var_4_ON_ = alarm_ON_;


// PRIVATE FUNCTIONS AND VARIABLES / CONSTANTS

private:

  // buzzer functions
  // buzzer used is a passive buzzer which is run using timers
  void SetupBuzzerTimer();
  static void IRAM_ATTR PassiveBuzzerTimerISR();
  void BuzzerEnable();
  void BuzzerDisable();

  // Hardware Timer
  hw_timer_t *passive_buzzer_timer_ptr_ = NULL;

  static inline uint16_t buzzer_frequency = 2048;
  static inline const unsigned long kBeepLengthMs = 800;

  static inline bool buzzer_square_wave_toggle_ = false;
  static inline bool beep_toggle_ = false;
  static inline unsigned long beep_start_time_ms_ = 0;

};


#endif     // ALARM_CLOCK_H
#pragma once

#include <stdint.h>
#include "led_builtin.h"
#include "led_status.hpp"
#include "system_time.h"

#define LED_CTRL_NUM 5 // built-in LED + 4 external LEDs
#define LED_CTRL_BRIGHTNESS_MAX 255
#define LED_CTRL_BRIGHTNESS_MIN 0

enum led_ctrl_name {
  LED_CONTROL_BUILT_IN = 0,
  LED_CONTROL_1,
  LED_CONTROL_2,
  LED_CONTROL_3,
  LED_CONTROL_4,
};

enum led_ctrl_mode {
  LED_OFF = 0,
  LED_ON,
  LED_BLINK_ONCE,
  LED_BLINK_REPEAT,
  LED_BRIGHTER,
  LED_DARKER,
  LED_BREATHING_ONCE,
  LED_BREATHING_REPEAT,
};

typedef struct led_control_t_ {
  enum led_ctrl_mode mode;
  uint8_t brightness;
  uint8_t current_brightness;
  time_ms_t blink_interval_ms;
  time_ms_t blink_last_update_ms;
  uint8_t breathing_step;
  time_ms_t breathing_last_update_ms;
  uint32_t breathing_direction; // 1 for increasing brightness, 0 for decreasing brightness
} led_control_t;

class ledControl {
  public:
    ledControl(ledStatus* led_ptr);

    void init(void);
    void update(void);

    void update_led(enum led_ctrl_name led_name);

    void set_mode(enum led_ctrl_name led_name, enum led_ctrl_mode mode);
    void set_brightness(enum led_ctrl_name led_name, uint8_t brightness); // for LED_ON, LED_BLINK, LED_BREATHING
    void set_blink_interval(enum led_ctrl_name led_name, time_ms_t interval_ms); // for LED_BLINK, LED_BREATHING
    void set_breathing_step(enum led_ctrl_name led_name, uint8_t step); // for LED_BREATHING

  private:
    ledStatus* led;
    time_ms_t current_time_ms;
    led_control_t led_controls[LED_CTRL_NUM];
};
#pragma once

#include <stdint.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"

class joystick {
  public:
  int stick_btn_state;
  int8_t x;
  int8_t y;

  joystick(int stick_x_pin, int stick_y_pin, bool invert_x = false, bool invert_y = false, int stick_btn_pin = -1);

  void init(void);
  void update(void);

  private:
    int _stick_btn_pin, _stick_x_pin, _stick_y_pin;
    uint _ch_x, _ch_y;
    bool _invert_x, _invert_y;
};
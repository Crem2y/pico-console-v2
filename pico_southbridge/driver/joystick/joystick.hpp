#pragma once

#include <stdint.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"

class joystick {
  public:
  int8_t x;
  int8_t y;

  joystick(int stick_x_pin, int stick_y_pin, uint ch_x, uint ch_y);

  void init(void);
  void update(void);

  private:
    int _stick_x_pin, _stick_y_pin;
    uint _ch_x, _ch_y;
};
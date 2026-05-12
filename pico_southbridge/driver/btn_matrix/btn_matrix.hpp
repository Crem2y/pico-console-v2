#pragma once

#include <stdint.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"

class btn_matrix {
  public:
  uint32_t btn_data;

  btn_matrix(int h1_pin, int h2_pin, int h3_pin, int h4_pin, int v1_pin, int v2_pin, int v3_pin, int v4_pin);

  void init(void);
  void update(void);

  uint32_t get_btn_data(void);

  private:
    int _h1_pin;
    int _h2_pin;
    int _h3_pin;
    int _h4_pin;
    int _v1_pin;
    int _v2_pin;
    int _v3_pin;
    int _v4_pin;
};
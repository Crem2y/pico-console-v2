#pragma once

#include <stdint.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"

class btn_matrix {
  public:
  int32_t btn_data;

  btn_matrix(int btn_pin); //placeholder

  void init(void);
  void update(void);

  int32_t get_btn_data(void);

  private:
    int _btn_pin;
};
#pragma once

#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"

class irRemote {
  public:
  irRemote(int ir_pin);

  void init(void);

  void manual_on(void);
  void manual_off(void);

  void send_nec_bit(uint8_t data);
  void send_nec_byte(uint8_t data);
  void send_nec_format(bool short_leader, uint8_t custom1, uint8_t custom2, uint8_t data);

  private:
    int _ir_pin;
    uint slice_num;
    int ir_pwm_ch;
    int ir_pwm_top;
};
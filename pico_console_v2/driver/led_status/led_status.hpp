#pragma once

#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"

#define LED_PWM_MAX 1000
#define LED_PWM_MIN 0
#define LED_NUM 4

class ledStatus {
  public:
    ledStatus(int led_1, int led_2, int led_3, int led_4);

    void init(void);
    void set_bright(uint32_t num, uint32_t bright);
    void set_bright_float(uint32_t num, float bright);
    void set_bright_all(uint32_t* bright_arr);
    uint32_t get_bright(uint32_t num);

  private:
    int led_pin[LED_NUM];
    uint slice_num[LED_NUM];
    int led_bright[LED_NUM];
    int led_pwm_ch[LED_NUM];
};
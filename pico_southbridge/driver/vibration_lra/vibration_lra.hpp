#pragma once

#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"

#define LRA_PWM_MAX 255
#define LRA_PWM_MIN 0
#define LRA_NUM 2
#define LRA_FREQ_MAX 1000

class vibrationLRA {
  public:
    vibrationLRA(int lra_l_pin, int lra_r_pin);

    void init(void);
    void set_power(uint32_t num, uint8_t power);
    void set_freq(uint32_t num, uint32_t freq);

    void enable(void);
    void disable(void);

    void update_from_bridge(uint8_t* data, uint8_t len);

    int lra_pin[LRA_NUM];
    uint slice_num[LRA_NUM];

    bool lra_enabled;
    uint8_t lra_power[LRA_NUM];
    int lra_pwm_ch[LRA_NUM];
    int lra_dulation_us[LRA_NUM];
};
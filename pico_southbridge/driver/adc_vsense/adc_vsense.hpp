#pragma once

#include "pico/stdlib.h"
#include "hardware/adc.h"

class AdcVSense {
  public:
    AdcVSense(int adc_pin, float res_ratio = 1, int32_t offset_mv = 0);

    void init(void);
    float read_voltage(void);
    float get_voltage(void) {
      return voltage;
    };

  private:
    int _adc_pin;
    int _adc_channel;
    float _ratio;
    int32_t _offset_mv;
    float voltage;
};
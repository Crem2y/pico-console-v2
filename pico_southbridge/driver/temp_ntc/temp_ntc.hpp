#pragma once

#include "pico/stdlib.h"
#include "hardware/adc.h"

class tempNTC {
  public:
    tempNTC(int ntc_pin, float r_upper = 10000);
    void init(void);

    float read_temperature(void);
    float get_temperature(void) {
      return temperature;
    }
    float get_resistance(void) {
      return resistance;
    }

  private:
    int _ntc_pin;
    int _adc_channel;
    float _r_upper;
    float _t0;
    uint32_t _r0;

    float resistance;
    float temperature;
};
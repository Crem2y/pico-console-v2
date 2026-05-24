#pragma once

#include "pico/stdlib.h"
#include "hardware/adc.h"

class tempNTC {
  public:
    tempNTC(int ntc_pin, float ratio = 0.5f, float t0 = 25.0f);
    void init(void);
    float read(void);

  private:
    int _ntc_pin;
    int _adc_channel;
    float _ratio;
    float _t0;
};
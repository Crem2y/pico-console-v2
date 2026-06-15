#pragma once

#include "pico/stdlib.h"
#include "hardware/adc.h"

class liBattery {
  public:
  float voltage;
  float level;
  
  liBattery(int adc_pin, float res_ratio);

  void init(void);
  float get_voltage(void);
  float get_level(void);

  private:
    int _adc_pin;
    int _adc_channel;
    float _res_ratio;
};
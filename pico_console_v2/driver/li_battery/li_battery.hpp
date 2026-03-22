#pragma once

#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"

#define BAT_ADC_CH 2

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
    float _res_ratio;
};
#include "li_battery.hpp"

liBattery::liBattery(int adc_pin, float res_ratio) {
  _adc_pin = adc_pin;
  _res_ratio = res_ratio;
}

void liBattery::init(void) {
  adc_init();
  adc_gpio_init(_adc_pin);
}

float liBattery::get_voltage(void) {
  adc_select_input(BAT_ADC_CH);
  uint16_t adc_raw = 0;
  
  for(int i=0; i<16; i++) {
    adc_raw += adc_read();
  }

  voltage = ((float)adc_raw * (3.3/65536)) / _res_ratio;
  return voltage;
}

float liBattery::get_level(void) {
  float bat_voltage = get_voltage();

  float bat_level = 0;
  if(bat_voltage >= 4.2) {
    bat_level = 100;
  } else if(bat_voltage > 4.1) { // 99.9 ~ 96.0, 100mV
    bat_level = (((bat_voltage - 4.1) / 0.1) * 4) + 96;
  } else if(bat_voltage > 3.8) { // 95.9 ~ 56.0, 300mV
    bat_level = (((bat_voltage - 3.8) / 0.3) * 40) + 56;
  } else if(bat_voltage > 3.4) { // 55.9 ~ 13.0, 400mV
    bat_level = (((bat_voltage - 3.4) / 0.4) * 43) + 13;
  } else if(bat_voltage > 3.0) { // 12.9 ~ 0.0, 400mV
    bat_level = (((bat_voltage - 3.0) / 0.4) * 13) + 0;
  } else {
    bat_level = 0;
  }
  level = bat_level;

  return level;
}
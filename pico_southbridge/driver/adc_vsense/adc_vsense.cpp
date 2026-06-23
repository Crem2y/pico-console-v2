#include "adc_vsense.hpp"

AdcVSense::AdcVSense(int adc_pin, float ratio, int32_t offset_mv) {
  _adc_pin = adc_pin;
  _adc_channel = adc_pin - ADC_BASE_PIN;
  _ratio = ratio;
  _offset_mv = offset_mv;
}

void AdcVSense::init(void) {
  adc_init();
  adc_gpio_init(_adc_pin);
  if(_ratio == 0) _ratio = 1;
}

float AdcVSense::read_voltage(void) {
  adc_select_input(_adc_channel);
  uint16_t adc_raw = 0;
  
  for(int i=0; i<16; i++) {
    adc_raw += adc_read();
  }

  voltage = (((float)adc_raw * (3.3f/(4095*16))) / _ratio) + _offset_mv;
  return voltage;
}
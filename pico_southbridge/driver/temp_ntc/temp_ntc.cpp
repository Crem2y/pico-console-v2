#include "temp_ntc.hpp"

tempNTC::tempNTC(int ntc_pin, float ratio, float t0) {
  _ntc_pin = ntc_pin;
  _adc_channel = ntc_pin - ADC_BASE_PIN;
  _ratio = ratio;
  _t0 = t0;
}

void tempNTC::init(void) {
  adc_init();
  adc_gpio_init(_adc_channel);
}

float tempNTC::read(void) {
  adc_select_input(_adc_channel);
  uint16_t ntc_raw = adc_read();
  return ntc_raw / 4095.0f * 3.3f; // test
}
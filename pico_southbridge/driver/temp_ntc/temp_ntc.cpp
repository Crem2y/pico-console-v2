#include "temp_ntc.hpp"

#define NTC_TABLE_LEN 4

static float ntc_table[NTC_TABLE_LEN][2] = {
  {20, 12573},
  {25, 10000}, // T0
  {30, 7996},
  {35, 6429}
};

tempNTC::tempNTC(int ntc_pin, float r_upper) {
  _ntc_pin = ntc_pin;
  _adc_channel = ntc_pin - ADC_BASE_PIN;
  _r_upper = r_upper;
}

void tempNTC::init(void) {
  adc_init();
  adc_gpio_init(_ntc_pin);
}

float tempNTC::read_temperature(void) {
  adc_select_input(_adc_channel);
  uint16_t ntc_raw = adc_read();
  float adc_v = ntc_raw / 4095.0f * 3.3f;

  resistance = _r_upper * ((3.3f - adc_v) / adc_v);

  if(resistance >= ntc_table[0][1]) {
    temperature = ntc_table[0][0];
    return temperature;
  }
  if (resistance <= ntc_table[NTC_TABLE_LEN-1][1]){
    temperature = ntc_table[NTC_TABLE_LEN-1][0];
    return temperature;
  }

  for(int i=0; i<NTC_TABLE_LEN-1; i++) {
    float t_hi_r = ntc_table[i][1];
    float t_lo_r = ntc_table[i+1][1];

    if(resistance < t_hi_r && resistance > t_lo_r) {
      float t0 = ntc_table[i][0];
      float r0 = ntc_table[i][1];
      float t1 = ntc_table[i+1][0];
      float r1 = ntc_table[i+1][1];

      float ratio = (resistance - r0) / (r1 - r0);
      temperature = t0 + ratio * (t1 - t0);
      return temperature;
    }
  }

  return ntc_table[NTC_TABLE_LEN-1][0];
}
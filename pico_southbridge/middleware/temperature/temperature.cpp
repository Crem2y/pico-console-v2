#include "temperature.hpp"

temperature::temperature(void) {
  for(int i=0; i<TEMP_CH_NUM; i++) {
    temp[i] = 0.0;
  }
}

void temperature::init(void) {
  built_in_temp_init();
}

void temperature::update(void) {
  temp[TEMP_BUILTIN] = (int32_t)(built_in_temp_read() * 100);
}

int temperature::make_bridge_payload(uint8_t* payload_buf, uint max_size) {
  if(max_size < TEMP_CH_NUM * 2) return -1;

  for(int i=0; i<TEMP_CH_NUM; i++) {
    payload_buf[i*2] = temp[i] & 0xFF; // temp_L
    payload_buf[i*2 + 1] = (temp[i] >> 8) & 0xFF; // temp_H
  }

  return TEMP_CH_NUM * 2;
}

temp_t temperature::get_temp(temp_ch ch) {
  if(ch < TEMP_CH_NUM) {
    return temp[ch];
  } else {
    return 0;
  }
}
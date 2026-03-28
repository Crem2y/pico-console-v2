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

void temperature::update_from_bridge(uint8_t* data, uint8_t len) {
  if(len < (TEMP_CH_NUM-1) * 2) return; // each temp is 2 bytes (int16_t)

  for(int i=0; i<TEMP_CH_NUM-1; i++) {
    temp[i+1] = (int16_t)((data[i*2+1] << 8) | data[i*2]);
  }
}

temp_t temperature::get_temp(temp_ch ch) {
  if(ch < TEMP_CH_NUM) {
    return temp[ch];
  } else {
    return 0;
  }
}
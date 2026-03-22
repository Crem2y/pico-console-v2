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

temp_t temperature::get_temp(temp_ch ch) {
  if(ch < TEMP_CH_NUM) {
    return temp[ch];
  } else {
    return 0;
  }
}
#include "temperature.hpp"
#include "bridge_protocol.hpp"

extern bridgeProtocol Bridge;

temperature::temperature(void) {
  for(int i=0; i<TEMP_CH_NUM; i++) {
    temp[i] = 0.0f;
  }
}

void temperature::init(void) {
  built_in_temp_init();
}

void temperature::update(void) {
  temp[TEMP_BUILTIN] = built_in_temp_read();

  send_bridge_data();
}

float temperature::get_temp(temp_ch ch) {
  if(ch < TEMP_CH_NUM) {
    return temp[ch];
  } else {
    return 0;
  }
}

void temperature::send_bridge_data(void) {
  int payload_size = TEMP_CH_NUM * 2;
  uint8_t payload_buf[PAYLOAD_MAX_SIZE];

  int16_t temp_x100 = temp[0] * 100.0f;

  payload_buf[0] = temp_x100 & 0xFF;
  payload_buf[1] = (temp_x100 >> 8) & 0xFF;

  Bridge.send(CMD_TEMPERATURE_DATA, payload_size, payload_buf);
}
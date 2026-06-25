#include "temperature.hpp"
#include "bridge_protocol.hpp"

extern bridgeProtocol Bridge;

temperature::temperature(tempNTC* ntc) {
  Ntc = ntc;
}

void temperature::init(void) {
  for(int i=0; i<TEMP_CH_NUM; i++) {
    temp[i] = 0.0f;
  }
  built_in_temp_init();
  Ntc->init();
}

void temperature::update(void) {
  temp[TEMP_BUILTIN] = built_in_temp_read();
  temp[TEMP_NTC] = Ntc->read_temperature();

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

  for(int i=0; i<TEMP_CH_NUM; i++) {
    int16_t temp_x100 = temp[i] * 100;
    payload_buf[i*2] = temp_x100 & 0xFF;
    payload_buf[i*2+1] = (temp_x100 >> 8) & 0xFF;
  }

  Bridge.send(CMD_TEMPERATURE_DATA, payload_size, payload_buf);
}
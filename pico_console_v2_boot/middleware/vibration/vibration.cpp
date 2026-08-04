#include "vibration.hpp"
#include "bridge_protocol.hpp"

extern bridgeProtocol Bridge;

vibration::vibration(void) {
}

void vibration::init(void) {
  for(int i=0; i<VIB_CH_NUM; i++) {
    data[i].freq = 0;
    data[i].power = 0;
  }
}

void vibration::update(void) {
  if(prev_data[VIB_L].freq == data[VIB_L].freq &&
     prev_data[VIB_L].power == data[VIB_L].power &&
     prev_data[VIB_R].freq == data[VIB_R].freq && 
     prev_data[VIB_R].power == data[VIB_R].power) {
    return;
  }

  send_bridge_vibration_data(data[VIB_L].freq, data[VIB_L].power, data[VIB_R].freq, data[VIB_R].power);

  prev_data[VIB_L].freq = data[VIB_L].freq;
  prev_data[VIB_L].power = data[VIB_L].power;
  prev_data[VIB_R].freq = data[VIB_R].freq;
  prev_data[VIB_R].power = data[VIB_R].power;
}

void vibration::set_vibration(enum vibration_channel channel, uint16_t freq, uint8_t power) {
  if(channel > VIB_R) return;
  data[channel].freq = freq;
  data[channel].power = power;
}

void vibration::set_freq(enum vibration_channel channel, uint16_t freq) {
  if(channel > VIB_R) return;
  data[channel].freq = freq;
}

void vibration::set_power(enum vibration_channel channel, uint8_t power) {
  if(channel > VIB_R) return;
  data[channel].power = power;
}

void vibration::send_bridge_vibration_data(uint16_t l_freq, uint8_t l_power, uint16_t r_freq, uint8_t r_power) {
  int payload_size = 6;
  uint8_t payload_buf[PAYLOAD_MAX_SIZE];

  payload_buf[0] = l_freq & 0xFF;
  payload_buf[1] = (l_freq >> 8) & 0xFF;
  payload_buf[2] = l_power;
  payload_buf[3] = r_freq & 0xFF;
  payload_buf[4] = (r_freq >> 8) & 0xFF;
  payload_buf[5] = r_power;

  Bridge.send(CMD_VIBRATION_DATA, payload_size, payload_buf);
}
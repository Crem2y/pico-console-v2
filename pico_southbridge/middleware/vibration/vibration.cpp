#include "vibration.hpp"

vibration::vibration(vibrationLRA* lra) {
  this->lra = lra;
}

void vibration::init() {
  lra->init();
}

void vibration::enable(bool enable) {
  if(enable) {
    lra->enable();
  } else {
    lra->disable();
  }
}

void vibration::recv_bridge_data(const uint8_t* payload, uint8_t payload_size) {
  if (payload_size < 6) return; // Not enough data

  uint32_t l_freq = payload[0] | (payload[1] << 8);
  uint8_t l_power = payload[2];
  uint32_t r_freq = payload[3] | (payload[4] << 8);
  uint8_t r_power = payload[5];

  lra->set_power(0, l_power);
  lra->set_freq(0, l_freq);
  lra->set_power(1, r_power);
  lra->set_freq(1, r_freq);
}
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

void vibration::update_from_bridge(uint8_t* data, uint8_t len) {
  //placeholder
  if (len < 6) return; // Not enough data

  uint32_t l_freq = data[0] | (data[1] << 8);
  uint8_t l_power = data[2];
  uint32_t r_freq = data[3] | (data[4] << 8);
  uint8_t r_power = data[5];

  lra->set_power(0, l_power);
  lra->set_freq(0, l_freq);
  lra->set_power(1, r_power);
  lra->set_freq(1, r_freq);
}
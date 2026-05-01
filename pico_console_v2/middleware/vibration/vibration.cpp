#include "vibration.hpp"

vibration::vibration(void) {
}

void vibration::init(void) {
  freq[VIBRATION_CH1] = 0;
  power[VIBRATION_CH1] = 0;
  freq[VIBRATION_CH2] = 0;
  power[VIBRATION_CH2] = 0;
}

void vibration::set_vibration(enum vibration_channel channel, uint16_t freq, uint8_t power) {
  if(channel > VIBRATION_CH2) return;
  this->freq[channel] = freq;
  this->power[channel] = power;
}

int vibration::make_bridge_payload(uint8_t* payload_buf, uint max_size) {
  if(max_size < 6) return -1; // need at least 6 bytes for 2 channels of vibration data

  // payload format: [channel1_freq(2 bytes), channel1_power(1 byte), channel2_freq(2 bytes), channel2_power(1 byte)]
  payload_buf[0] = (freq[VIBRATION_CH1] >> 8) & 0xFF; // high byte of channel 1 frequency
  payload_buf[1] = freq[VIBRATION_CH1] & 0xFF;        // low byte of channel 1 frequency
  payload_buf[2] = power[VIBRATION_CH1];               // channel 1 power
  payload_buf[3] = (freq[VIBRATION_CH2] >> 8) & 0xFF; // high byte of channel 2 frequency
  payload_buf[4] = freq[VIBRATION_CH2] & 0xFF;        // low byte of channel 2 frequency
  payload_buf[5] = power[VIBRATION_CH2];               // channel 2 power

  return 6; // total payload size
}
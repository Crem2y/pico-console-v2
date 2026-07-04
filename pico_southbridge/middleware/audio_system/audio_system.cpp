#include "audio_system.hpp"

audioSystem::audioSystem(void) {

}

void audioSystem::init(void) {

}

void audioSystem::recv_bridge_note_data(const uint8_t* payload, uint8_t payload_size) {
  if(payload_size < 6) return;

  float freq;
  memcpy(&freq, &payload[1], sizeof(float));
  voice_note_on(payload[0], freq, payload[5]);
}

void audioSystem::recv_bridge_set_wave(const uint8_t* payload, uint8_t payload_size) {
  if(payload_size < 2) return;

  set_voice_waveform(payload[0], (wave_t)payload[1]);
}

void audioSystem::recv_bridge_set_env(const uint8_t* payload, uint8_t payload_size) {
  if(payload_size < 6) return;

  uint32_t tick_us;
  memcpy(&tick_us, &payload[1], sizeof(uint32_t));
  voice_vol_env_set(payload[0], tick_us, payload[5]);
}

void audioSystem::recv_bridge_set_pitch_env(const uint8_t* payload, uint8_t payload_size) {
  if(payload_size < 7) return;

  uint32_t tick_us;
  memcpy(&tick_us, &payload[1], sizeof(uint32_t));
  voice_pitch_env_set(payload[0], tick_us, (int8_t)payload[5], payload[6]);
}

void audioSystem::recv_bridge_set_master(const uint8_t* payload, uint8_t payload_size) {
  if(payload_size < 1) return;

  set_master_volume(payload[0]);
}
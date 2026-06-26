#include "audio_system.hpp"

audioSystem::audioSystem(void) {

}

void audioSystem::init(void) {

}

void audioSystem::recv_bridge_data(const uint8_t* payload, uint8_t payload_size) {
  if(payload_size < 5) return; //placeholder

  float freq;
  memcpy(&freq, &payload[1], sizeof(float));
  voice_note_on(payload[0], freq, g_voices[0].vol_q8);
}
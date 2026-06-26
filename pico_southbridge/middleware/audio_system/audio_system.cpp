#include "audio_system.hpp"

audioSystem::audioSystem(void) {

}

void audioSystem::init(void) {

}

void audioSystem::recv_bridge_data(const uint8_t* payload, uint8_t payload_size) {
  if(payload_size < 3) return; //placeholder

  float freq = sound_freq_table[payload[1]][payload[2]];
  voice_note_on(payload[0], freq, g_voices[0].vol_q8);
}
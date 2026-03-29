#include "audio_system.hpp"

audioSystem::audioSystem(void) {

}

void audioSystem::init(void) {

}

void audioSystem::update_from_bridge(uint8_t* data, uint8_t len) {
  if(len < 3) return; //placeholder

  voice_note_on(data[0], data[1], data[2], g_voices[0].vol_q8);
}
#include "audio_system.hpp"

audioSystem::audioSystem(void) {

}

void audioSystem::init(void) {

}

void audioSystem::update_from_bridge(uint8_t* data, uint8_t len) {
  if(len < 2) return; //placeholder

  int octave = data[0]; // upper 4 bits for octave
  int note = data[1]; // lower 4 bits for note
  voice_note_on(0, octave, note, g_voices[0].vol_q8);
}
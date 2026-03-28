#include "audio_system.hpp"

audioSystem::audioSystem(void) {

}

void audioSystem::init(void) {
  current_note = NULL;
  current_note_index = 0;
}

void audioSystem::update(void) {
  current_time_ms = get_system_time_ms();

  if(current_note == NULL) return;

  if(system_time_elapsed_ms(current_time_ms, prev_note_time_ms) > current_note_duration_ms) {
    prev_note_time_ms = current_time_ms;

    if(current_note_index >= current_note_len) {
      current_note = NULL;
      current_note_index = 0;
      note_now = (music_note_t){0, 0}; // no sound
      return;
    }

    note_now = current_note[current_note_index];
    current_note_index++;
  } else {
    note_now = (music_note_t){0, 0}; // no sound
  }
}

int audioSystem::make_bridge_payload(uint8_t* payload_buf, uint max_size) {
  if(max_size < 1) return -1;

  if(note_now.octave == 0 && note_now.note == 0) {
    return 0; // no update
  }

  payload_buf[0] = note_now.octave;
  payload_buf[1] = note_now.note;

  return 2;
}

void audioSystem::play_music(music_table_t* music_table) {
  if(music_table->notes == NULL || music_table->len == 0) return;

  current_note = music_table->notes;
  current_note_len = music_table->len;
  current_note_index = 0;
  current_note_duration_ms = music_table->note_duration_ms;

  prev_note_time_ms = current_time_ms;
}
#pragma once

#include <stdint.h>
#include "system_time.h"

typedef struct _music_note_t { //placeholder
  uint8_t octave;
  uint8_t note;
} music_note_t;

typedef struct _music_table_t { //placeholder
  uint32_t len;
  time_ms_t note_duration_ms;
  music_note_t* notes;
} music_table_t;

class audioSystem {
  public:
    audioSystem(void);

    void init(void);
    void update(void);

    int make_bridge_payload(uint8_t* payload_buf, uint max_size);

    void play_music(music_table_t* music_table); //placeholder

  private:
    time_ms_t current_time_ms;
    time_ms_t prev_note_time_ms;

    music_note_t note_now;

    music_note_t* current_note;
    uint32_t current_note_len;
    uint32_t current_note_index;
    time_ms_t current_note_duration_ms;
};
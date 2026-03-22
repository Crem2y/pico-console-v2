#pragma once

#include <stdint.h>
#include <stdbool.h>

// -------------------- Configuration --------------------
#define WAVE_TABLE_LEN      2048
#define SAMPLES_PER_BUFFER  256
#define NUM_CHANNELS        16

// Choose output sample rate here (must match audio_format.sample_freq)
#define AUDIO_FS_HZ         24000

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    WAVE_SQUARE_12 = 0,
    WAVE_SQUARE_25,
    WAVE_SQUARE_50,
    WAVE_SQUARE_75,
    WAVE_TRIANGLE,
    WAVE_SAWTOOTH,
    WAVE_NOISE,
    WAVE_SINE,
} wave_t;

typedef struct {
    // Phase accumulator
    uint32_t pos;
    uint32_t step;

    // Base volume (Q8: 0..256 means 0.0..1.0)
    // This is the peak level when a note is triggered.
    int32_t vol_q8;

    // Envelope level (Q8). Applied volume is env_q8.
    // For now: simple linear decay envelope.
    int32_t env_q8;
    uint32_t env_tick_us;       // how often to update env (e.g., 5000)
    uint32_t env_next_us;       // next update time (time_us_32())
    int32_t env_decay_step_q8;  // amount to subtract each tick (>=1)

    // Current waveform table
    wave_t wave;
    const int16_t *table;
} voice_t;

extern const float sound_freq_table[9][12];
extern voice_t g_voices[NUM_CHANNELS];

void audio_loop(void);
void audio_init(int data_pin, int clock_pin_base);

void voice_note_on(int voice_idx, int octave_0_to_8, int semitone_0_to_11, int32_t peak_vol_q8);
void voice_env_set(voice_t *v, uint32_t tick_us, int32_t decay_step_q8);
void set_voice_waveform(int voice_idx, wave_t w);

#ifdef __cplusplus
}
#endif
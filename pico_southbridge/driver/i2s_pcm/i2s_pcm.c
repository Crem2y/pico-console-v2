#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>

#include "pico/stdlib.h"
#include "pico/audio_i2s.h"

#include "i2s_pcm.h"

static int16_t square_12_wave_table[WAVE_TABLE_LEN];
static int16_t square_25_wave_table[WAVE_TABLE_LEN];
static int16_t square_50_wave_table[WAVE_TABLE_LEN];
static int16_t square_75_wave_table[WAVE_TABLE_LEN];
static int16_t triangle_wave_table[WAVE_TABLE_LEN];
static int16_t sawtooth_wave_table[WAVE_TABLE_LEN];
static int16_t noise_wave_table[WAVE_TABLE_LEN];
static int16_t sine_wave_table[WAVE_TABLE_LEN];

static int _data_pin;
static int _clock_pin_base;

// -------------------- Wave tables --------------------
static void make_wave_tables(void) {
    int16_t noise_temp = -32768;

    for (int i = 0; i < WAVE_TABLE_LEN; i++) {
        square_12_wave_table[i] = (i < WAVE_TABLE_LEN / 8) ? 32767 : -32768;
        square_25_wave_table[i] = (i < WAVE_TABLE_LEN / 4) ? 32767 : -32768;
        square_50_wave_table[i] = (i < WAVE_TABLE_LEN / 2) ? 32767 : -32768;
        square_75_wave_table[i] = (i < (WAVE_TABLE_LEN * 3) / 4) ? 32767 : -32768;
        // triangle ranges -32768..32767 without overflow
        if (i < WAVE_TABLE_LEN / 2) {
            triangle_wave_table[i] = (int16_t)(((i * 65535) / (WAVE_TABLE_LEN / 2)) - 32768);
        } else {
            triangle_wave_table[i] = (int16_t)((((WAVE_TABLE_LEN - i) * 65535) / (WAVE_TABLE_LEN / 2)) - 32768);
        }
        sawtooth_wave_table[i] = (int16_t)((( (WAVE_TABLE_LEN - i) * 65535) / WAVE_TABLE_LEN) - 32768);
        if ((i+1) % 8 == 0) noise_temp = (rand() & 1) ? 32767 : -32768;
        noise_wave_table[i] = noise_temp;
        sine_wave_table[i] = (int16_t)(32767.0f * cosf((float)i * 2.0f * (float)(M_PI / WAVE_TABLE_LEN)));
    }
}

// -------------------- Note -> step table --------------------
static uint32_t note_step_table[9][12]; // octave 0..8

static inline uint32_t step_from_hz(float f_hz, uint32_t fs_hz) {
    // step = round( (WAVE_TABLE_LEN * 65536 * f) / Fs )
    // Use double for precision; called only on note changes.
    const double num = (double)WAVE_TABLE_LEN * 65536.0 * (double)f_hz;
    double step = num / (double)fs_hz;
    if (step < 0.0) step = 0.0;
    if (step > 4294967295.0) step = 4294967295.0;
    return (uint32_t)(step + 0.5);
}

static void build_note_step_table(uint32_t fs_hz) {
    for (int o = 0; o < 9; o++) {
        for (int n = 0; n < 12; n++) {
            note_step_table[o][n] = step_from_hz(sound_freq_table[o][n], fs_hz);
        }
    }
}

// -------------------- Simple synth state --------------------

voice_t g_voices[NUM_CHANNELS];

void voice_env_set(voice_t *v, uint32_t tick_us, int32_t decay_step_q8) {
    v->env_tick_us = tick_us;
    v->env_decay_step_q8 = (decay_step_q8 < 0) ? 1 : decay_step_q8;
    v->env_next_us = time_us_32() + v->env_tick_us;
}

static inline void voice_env_init(voice_t *v, uint32_t tick_us, int32_t decay_step_q8) {
    voice_env_set(v, tick_us, decay_step_q8);
    v->env_q8 = 0;
}

static inline void voice_env_note_on(voice_t *v, int32_t peak_vol_q8) {
    if (peak_vol_q8 < 0) peak_vol_q8 = 0;
    if (peak_vol_q8 > 256) peak_vol_q8 = 256;
    v->vol_q8 = peak_vol_q8;
    v->env_q8 = peak_vol_q8;
    v->env_next_us = time_us_32() + v->env_tick_us;
}

static inline void voice_env_tick(voice_t *v, uint32_t now_us) {
    // Simple linear decay: env_q8 -= env_decay_step_q8 every env_tick_us
    if (v->env_q8 <= 0) return;

    // Catch up if we missed ticks (avoid depending on main loop cadence)
    while ((int32_t)(now_us - v->env_next_us) >= 0) {
        v->env_q8 -= v->env_decay_step_q8;
        if (v->env_q8 <= 0) {
            v->env_q8 = 0;
            break;
        }
        v->env_next_us += v->env_tick_us;
    }
}

static inline int32_t voice_next_sample_i32(voice_t *v) {
    const uint32_t pos_max = 0x10000u * (uint32_t)WAVE_TABLE_LEN;

    const int32_t s = (int32_t)v->table[v->pos >> 16u];
    // Apply envelope level (Q8)
    int32_t y = (s * v->env_q8) >> 8;

    v->pos += v->step;
    if (v->pos >= pos_max) v->pos -= pos_max;

    return y;
}

static void render_buffer_mono_mix(int16_t *dst, uint32_t count) {
    uint32_t now_us = time_us_32();
    // Update envelopes once per buffer render (cheap). If you need tighter timing,
    // call voice_env_tick() inside the sample loop instead.
    for (int v = 0; v < NUM_CHANNELS; v++) {
        voice_env_tick(&g_voices[v], now_us);
    }
    for (uint32_t i = 0; i < count; i++) {
        int32_t acc = 0;
        // 4 fixed voices mixed into mono
        for (int v = 0; v < NUM_CHANNELS; v++) {
            acc += voice_next_sample_i32(&g_voices[v]);
        }

        // Simple headroom to reduce clipping when multiple voices stack.
        // For NUM_CHANNELS=4, shifting by 2 approximates /4.
        acc >>= 2;

        // Clip to int16 range
        if (acc > 32767) acc = 32767;
        if (acc < -32768) acc = -32768;
        // Left channel
        dst[i * 2 + 0] = (int16_t)acc;
        // Right channel
        dst[i * 2 + 1] = (int16_t)acc;
    }
}

static const int16_t *wave_table_ptr(wave_t w) {
    switch (w) {
        case WAVE_SQUARE_12:    return square_12_wave_table;
        case WAVE_SQUARE_25:    return square_25_wave_table;
        case WAVE_SQUARE_50:    return square_50_wave_table;
        case WAVE_SQUARE_75:    return square_75_wave_table;
        case WAVE_TRIANGLE:     return triangle_wave_table;
        case WAVE_SAWTOOTH:     return sawtooth_wave_table;
        case WAVE_NOISE:        return noise_wave_table;
        case WAVE_SINE:         return sine_wave_table;
        default:                return square_50_wave_table;
    }
}

void set_voice_waveform(int voice_idx, wave_t w) {
    if (voice_idx < 0 || voice_idx >= NUM_CHANNELS) return;
    g_voices[voice_idx].table = wave_table_ptr(w);
    g_voices[voice_idx].wave = w;
}

static bool set_voice_note(int voice_idx, int octave_0_to_8, int semitone_0_to_11) {
    if (voice_idx < 0 || voice_idx >= NUM_CHANNELS) return false;
    if (octave_0_to_8 < 0 || octave_0_to_8 > 8) return false;
    if (semitone_0_to_11 < 0 || semitone_0_to_11 > 11) return false;
    g_voices[voice_idx].step = note_step_table[octave_0_to_8][semitone_0_to_11];
    return true;
}

static void set_voice_volume_q8(int voice_idx, int32_t vol_q8) {
    if (voice_idx < 0 || voice_idx >= NUM_CHANNELS) return;
    if (vol_q8 < 0) vol_q8 = 0;
    if (vol_q8 > 256) vol_q8 = 256;
    g_voices[voice_idx].vol_q8 = vol_q8;
    // Do not directly change env_q8 here; env_q8 is controlled by note triggers.
}

void voice_note_on(int voice_idx, int octave_0_to_8, int semitone_0_to_11, int32_t peak_vol_q8) {
    if (!set_voice_note(voice_idx, octave_0_to_8, semitone_0_to_11)) return;
    voice_env_note_on(&g_voices[voice_idx], peak_vol_q8);
}

// -------------------- Audio init --------------------
static struct audio_buffer_pool *init_audio(void) {
    static audio_format_t audio_format = {
        .format = AUDIO_BUFFER_FORMAT_PCM_S16,
        .sample_freq = AUDIO_FS_HZ,
        .channel_count = 2,
    };

    static struct audio_buffer_format producer_format = {
        .format = &audio_format,
        .sample_stride = 4,
    };

    struct audio_buffer_pool *producer_pool = audio_new_producer_pool(&producer_format,
                                                                      3,
                                                                      SAMPLES_PER_BUFFER);
    bool ok;

    struct audio_i2s_config config = {
        .data_pin = _data_pin,
        .clock_pin_base = _clock_pin_base,
        .dma_channel = 0,
        .pio_sm = 0,
    };

    const struct audio_format *output_format = audio_i2s_setup(&audio_format, &config);
    if (!output_format) {
        panic("PicoAudio: Unable to open audio device\n");
    }

    ok = audio_i2s_connect(producer_pool);
    assert(ok);
    audio_i2s_set_enabled(true);

    return producer_pool;
}


void audio_loop(void) {

    struct audio_buffer_pool *ap = init_audio();

    while(true) {
        struct audio_buffer *buffer = take_audio_buffer(ap, true);
        int16_t *samples = (int16_t *)buffer->buffer->bytes;

        render_buffer_mono_mix(samples, buffer->max_sample_count);

        buffer->sample_count = buffer->max_sample_count;
        give_audio_buffer(ap, buffer);
    }
}

/**
 * @brief 
 * 
 * @param data_pin i2s data pin
 * @param clock_pin_base i2s clock pin base (clock_pin_base = bit clock, clock_pin_base+1 = word select)
 * @return struct audio_buffer_pool* 
 */
void audio_init(int data_pin, int clock_pin_base) {
    _data_pin = data_pin;
    _clock_pin_base = clock_pin_base;
    make_wave_tables();
    build_note_step_table(AUDIO_FS_HZ);

    memset(g_voices, 0, sizeof(g_voices));

    for (int i = 0; i < NUM_CHANNELS; i++) {
        set_voice_waveform(i, WAVE_SQUARE_50);
        set_voice_volume_q8(i, 8);
        voice_env_init(&g_voices[i], 100000, 1);
    }
}
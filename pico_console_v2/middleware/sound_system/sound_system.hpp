#pragma once
#include <stdint.h>
#include "pico/stdlib.h"

#define DAC_HW_MAX 4095
#define DAC_CH_MAX 4
#define DAC_VOL_MAX 255

#define WAVE_RES 10000    // 계산 정확도
#define WAVE_FREQ 10000.0 // 샘플링 주파수

#define USING_CH 4

volatile typedef struct _dac_set_t {
  float         volume;     // dac volume (0~1)
  unsigned int  wave_count; // dac wave count (0~127)
  unsigned int  count;      // dac count (x1000)
  unsigned int  count_up;   // dac count up value (x1000)
  const float*  wave;       // dac wave (pointer)
} dac_set_t;

enum wave {
  W_NONE = 0,
  W_PS25,
  W_PS50,
  W_PS75,
  W_TRIN,
  W_SAWT,
  W_SINE,
  W_NOIZ,
};

enum note {
  S_NN = 0,
  S_C, S_Cs,
  S_D, S_Ds,
  S_E,
  S_F, S_Fs,
  S_G, S_Gs,
  S_A, S_As,
  S_B
};

enum op_code {
  OP_NONE = 0x00,
  OP_DELAY1,
  OP_DELAY2,
  OP_DELAY3,
  OP_DELAY4,
  OP_MUTE = 0x10,
  OP_STOP = 0xFF
};

class soundSystem {
  public:
    soundSystem(void);

    void init(void (*dac_output)(uint16_t left, uint16_t right), void (*dac_mute)(void), void (*dac_unmute)(void));
    void init_timer(void);
    void manual_beep(uint32_t ms);
    void manual_mute(void);

    void beep(uint32_t ms);
    void set_mute(bool mute);
    void set_waning(uint32_t waning);
    void sound_waning(void);

    void delay(uint32_t ms);
    void channel_set(uint8_t channel, float freq, uint8_t volume);
    void channel_wave(uint8_t channel, float freq, uint8_t volume, const float* wave);
    void channel_stop(uint8_t channel);

    void play_sound(uint16_t (*music_table)[USING_CH]);
    void play_music(uint16_t (*music_table)[USING_CH], uint32_t music_length, uint32_t music_delay);

    void play_sound_ex(const uint32_t music_table);
    void play_music_ex(const uint32_t *music_table, uint32_t music_length, uint32_t music_delay);

    static bool timer_callback_sound(struct repeating_timer *t);
    static int64_t delay_callback(alarm_id_t id, void *user_data);

    static void (*dac_output)(uint16_t left, uint16_t right);
    static void (*dac_mute)(void);
    static void (*dac_unmute)(void);

  private:
    int music_table_pos;
    float _sound_waning;
};
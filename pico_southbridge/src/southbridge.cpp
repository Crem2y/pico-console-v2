#include <stdio.h>

// pico libraries
#include "pico/stdlib.h"
#include "pico/multicore.h"

// headers
#include "southbridge.hpp"

// drivers
// #include "uart_log.h"
#include "uart_bridge.h"
#include "i2s_pcm.h"
// #include "li_battery.hpp"
// #include "ir_remote.hpp"
#include "system_time.h"
#include "temp_builtin.h"

// middlewares
#include "audio_system.hpp"
// #include "gamepad.hpp"
#include "temperature.hpp"

// // hw lib init
// pca9554 Key = pca9554(i2c1, 3,2);
// liBattery Bat = liBattery(28, ((float)1/2));
// irRemote Ir = irRemote(21);

// // middleware lib init
// ledControl LedCtrl = ledControl(&Led);
// gamepad Gamepad = gamepad(&Key);
audioSystem Audio = audioSystem();
temperature Temperature = temperature();

static const char *note_names[13] = {
    "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"
};

static const char *wave_names[8] = {
    "Square 12", "Square 25", "Square 50", "Square 75", "Triangle", "Sawtooth", "Noise", "Sine"
};

void core1_entry(void);
void bridge_do_cmd(bridge_protocol_t* cmd);

void audio_control(void);

//////// function ////////

int main() {
//  uartLog_init(uart0, 0, 1, 115200);
  stdio_init_all();
  uart_bridge_init(uart1, 4, 5, 115200);
  set_bridge_do_cmd(bridge_do_cmd);

  audio_init(6, 7);
  
  gpio_init(29);
  gpio_set_dir(29, GPIO_IN);
  gpio_pull_up(29);

  sleep_ms(100);

  multicore_launch_core1(core1_entry);

  // boot sound
  // voice_note_on(0, 6, 0, g_voices[0].vol_q8);
  // sleep_ms(100);
  // voice_note_on(0, 7, 0, g_voices[0].vol_q8);
  // sleep_ms(100);
  getchar_timeout_us(100);

//  LOG_PRINTF("go to main loop\n");
//  multicore_fifo_push_blocking(1);

  sleep_ms(100);
  // boot sequence end
  
  // Gamepad.update();

  int btn_status_old = 0;
  while (true) {
    //sleep_ms(10);
    audio_control();
    bridge_handle();

    int btn_status = gpio_get(29);
    if(btn_status != btn_status_old) {
      btn_status_old = btn_status;

      bridge_protocol_t response_cmd;
      uint8_t temp_payload[PAYLOAD_MAX_SIZE];
      temp_payload[0] = !btn_status;
      response_cmd = bridge_protocol_create(CMD_GAMEPAD_DATA, 1, temp_payload);
      bridge_cmd_queue_push(response_cmd);
    }
    // Gamepad.update();
    // Bat.get_level();
    // LedCtrl.update();
    // Temperature.update();
  }

  return 0;
}

void core1_entry(void) {

//  multicore_fifo_pop_blocking(); // wait until boot process is done

  while (true) {
    //sleep_ms(10);
    audio_loop();
  }
}

void audio_control(void) {
    static int base_octave = 4;
    int octave = base_octave;
    static int note = 9; // A
    static int channel = 0;
    static wave_t current_wave = WAVE_SQUARE_50;
    wave_t new_wave = current_wave;

    // Map keys a..k to 12 semitones (simple layout)2
    // a:C  w:C#  s:D  e:D#  d:E  f:F  t:F#  g:G  y:G#  h:A  u:A#  j:B

    int c = getchar_timeout_us(0);
    if (c >= 0) {
        switch(c) {
            // Octave (VOICE0)
            case 'z':
                if (base_octave > 0) base_octave--;
                break;
            case 'x':
                if (base_octave < 8) base_octave++;
                break;
            // Waveform (VOICE0)
            case '1':
                new_wave = WAVE_SQUARE_12;
                break;
            case '2':
                new_wave = WAVE_SQUARE_25;
                break;
            case '3':
                new_wave = WAVE_SQUARE_50;
                break;
            case '4':
                new_wave = WAVE_SQUARE_75;
                break;
            case '5':
                new_wave = WAVE_TRIANGLE;
                break;
            case '6':
                new_wave = WAVE_SAWTOOTH;
                break;
            case '7':
                new_wave = WAVE_NOISE;
                break;
            case '8':
                new_wave = WAVE_SINE;
                break;
            // Volume (VOICE0)
            case '[':
                if (g_voices[0].vol_q8 > 0) g_voices[0].vol_q8 -= 4;
                break;
            case ']':
                if (g_voices[0].vol_q8 < 256) g_voices[0].vol_q8 += 4;
                break;
            // Notes (VOICE0)
            case 'a':
                note = 0;
                break;
            case 'w':
                note = 1;
                break;
            case 's':
                note = 2;
                break;
            case 'e':
                note = 3;
                break;
            case 'd':
                note = 4;
                break;
            case 'f':
                note = 5;
                break;
            case 't':
                note = 6;
                break;
            case 'g':
                note = 7;
                break;
            case 'y':
                note = 8;
                break;
            case 'h':
                note = 9;
                break;
            case 'u':
                note = 10;
                break;
            case 'j':
                note = 11;
                break;
            // Notes (VOICE0)
            case 'k':
                if (octave < 8) octave++;
                note = 0;
                break;
            case 'o':
                if (octave < 8) octave++;
                note = 1;
                break;
            case 'l':
                if (octave < 8) octave++;
                note = 2;
                break;
            case 'p':
                if (octave < 8) octave++;
                note = 3;
                break;
            case ';':
                if (octave < 8) octave++;
                note = 4;
                break;
            default:
                break;
        }

        // Trigger a new note on VOICE0: reset envelope to current base volume
        voice_note_on(channel, octave, note, g_voices[channel].vol_q8);

        // channel++;
        // if (channel >= NUM_CHANNELS) channel = 0;

        if (new_wave != current_wave) {
            for (int i = 0; i < NUM_CHANNELS; i++) {
                set_voice_waveform(i, new_wave);
            }
        }

        printf("V0=%s %s%d Hz=%.3f vol_q8=%ld | Fixed: V1=A3 V2=C4 V3=E4      \r",
            wave_names[g_voices[0].wave],
            note_names[note], octave,
            sound_freq_table[octave][note],
            (long)g_voices[0].vol_q8);
    }
}

void bridge_do_cmd(bridge_protocol_t* cmd) {
  enum bridge_cmd command = cmd->cmd;
  switch (command)
  {
  case CMD_AUDIO_PCM_DATA:
  {
    //placeholder for audio data handling
    int octave = (cmd->payload[0] >> 4) & 0x0F; // upper 4 bits for octave
    int note = cmd->payload[0] & 0x0F; // lower 4 bits for note
    voice_note_on(0, octave, note, g_voices[0].vol_q8);
    break;
  }
  default:
    break;
  }
}
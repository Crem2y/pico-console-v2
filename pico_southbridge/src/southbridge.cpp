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

void core1_entry(void);
void bridge_do_cmd(bridge_protocol_t* cmd);

//////// function ////////

int main() {
//  uartLog_init(uart0, 0, 1, 115200);
  stdio_init_all();
  uart_bridge_init(uart1, 4, 5, 921600);
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
    bridge_handle();

    //placeholder
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
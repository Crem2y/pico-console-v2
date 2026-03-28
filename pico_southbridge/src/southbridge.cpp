// headers
#include "southbridge.hpp"

// // hw lib init
// liBattery Bat = liBattery(28, ((float)1/2));
// irRemote Ir = irRemote(21);
btn_matrix BtnMatrix = btn_matrix(28);
joystick Joy1 = joystick(26, 27, 0, 1);

// // middleware lib init
gamepad Gamepad = gamepad(&BtnMatrix, &Joy1, NULL);
audioSystem Audio = audioSystem();
temperature Temperature = temperature();

void core1_entry(void);
void bridge_do_cmd(bridge_protocol_t* cmd);

time_ms_t gamepad_timer;
time_ms_t temperature_timer;

//////// function ////////

int main() {
//  uartLog_init(uart0, 0, 1, 115200);
  stdio_init_all();
  uart_bridge_init(uart1, 4, 5, 921600);
  set_bridge_do_cmd(bridge_do_cmd);

  audio_init(6, 7);
  Audio.init();
  Gamepad.init();
  Temperature.init();

  sleep_ms(100);

  multicore_launch_core1(core1_entry);
  uart_bridge_enable_irq();

  // boot sequence end

  while (true) {
    time_ms_t now_time = get_system_time_ms();
    bridge_protocol_t response_cmd; 
    uint8_t temp_payload[PAYLOAD_MAX_SIZE];
    int payload_size = 0;

    bridge_handle();

    if(system_time_elapsed_ms(now_time, gamepad_timer) > 10) {
      gamepad_timer = now_time;
      Gamepad.update();
      payload_size = Gamepad.make_bridge_payload(temp_payload, PAYLOAD_MAX_SIZE);
      if(payload_size > 0) {
        response_cmd = bridge_protocol_create(CMD_GAMEPAD_DATA, payload_size, temp_payload);
        bridge_cmd_queue_push(response_cmd);
      }
    }
    if(system_time_elapsed_ms(now_time, temperature_timer) > 1000) {
      temperature_timer = now_time;
      Temperature.update();
      payload_size = Temperature.make_bridge_payload(temp_payload, PAYLOAD_MAX_SIZE);
      if(payload_size > 0) {
        response_cmd = bridge_protocol_create(CMD_TEMPERATURE_DATA, payload_size, temp_payload);
        bridge_cmd_queue_push(response_cmd);
      }
    }
    // Bat.get_level();
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
  case CMD_AUDIO_ENABLE:
    // Audio.enable();
    break;
  case CMD_AUDIO_PCM_DATA:
    Audio.update_from_bridge(cmd->payload, cmd->payload_size);
    break;
  case CMD_AUDIO_DISABLE:
    // Audio.disable();
    break;
  default:
    break;
  }
}
// headers
#include "southbridge.hpp"

// hw lib init
btn_matrix BtnMatrix = btn_matrix(PIN_BTN_H1, PIN_BTN_H2, PIN_BTN_H3, PIN_BTN_H4, PIN_BTN_V1, PIN_BTN_V2, PIN_BTN_V3, PIN_BTN_V4);
joystick Joy1 = joystick(PIN_JOY1_X, PIN_JOY1_Y, false, false, PIN_JOY1_BTN);
joystick Joy2 = joystick(PIN_JOY2_X, PIN_JOY2_Y, true,  true,  PIN_JOY2_BTN);
vibrationLRA Lra = vibrationLRA(PIN_LRA_L, PIN_LRA_R);
bq25619 Charger = bq25619(i2c1, PIN_I2C_SDA, PIN_I2C_SCL, PIN_BAT_INT);
mpu6050 Mpu = mpu6050(i2c1, PIN_I2C_SDA, PIN_I2C_SCL, PIN_IMU_INT);

// middleware lib init
gamepad Gamepad = gamepad(&BtnMatrix, &Joy1, &Joy2);
audioSystem Audio = audioSystem();
temperature Temperature = temperature();
vibration Vibration = vibration(&Lra);

void core1_entry(void);
void bridge_do_cmd(bridge_protocol_t* cmd);

time_ms_t gamepad_timer;
time_ms_t temperature_timer;
time_ms_t imu_timer;
time_ms_t battery_timer;

//////// function ////////

int main() {
//  uartLog_init(uart0, 0, 1, 115200);
  stdio_init_all();
  uart_bridge_init(uart1, PIN_BRIDGE_TX, PIN_BRIDGE_RX, 921600);
  set_bridge_do_cmd(bridge_do_cmd);

  audio_init(PIN_I2S_DATA, PIN_I2S_SCK);
  Audio.init();
  // Audio.enable();
  gpio_init(PIN_DAC_MUTE); //placeholder for dac mute pin
  gpio_set_dir(PIN_DAC_MUTE, GPIO_OUT);
  gpio_put(PIN_DAC_MUTE, 1);
  Gamepad.init();
  Temperature.init();
  Vibration.init();
  Vibration.enable(true);
  Charger.init();
  Mpu.init();

  sleep_ms(100);

  multicore_launch_core1(core1_entry);
  uart_bridge_enable_irq();

  // boot sequence end
  // Charger.set_ignore_ts(true); //test
  Charger.read_all_regs();
  for(int i=0; i<13; i++) { //test
    printf("bq25619 reg 0x%02X : 0x%02X\n", i, Charger.reg_raw[i]);
  }

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
    if(system_time_elapsed_ms(now_time, imu_timer) > 100) { //placeholder
      imu_timer = now_time;
      Mpu.read_raw_accel_data();
      Mpu.read_raw_gyro_data();
      Mpu.read_raw_temp_data();

      // printf("Accel: X=%6d Y=%6d Z=%6d | Gyro: X=%6d Y=%6d Z=%6d | ", Mpu.accel_raw.x, Mpu.accel_raw.y, Mpu.accel_raw.z, Mpu.gyro_raw.x, Mpu.gyro_raw.y, Mpu.gyro_raw.z);
      // printf("Temp. = % 3.1fC\n", (Mpu.temp_raw / 340.0) + 36.53f); // temperature formula from pico-examples/i2c/mpu6050_i2c/mpu6050_i2c.c
    }
    if(system_time_elapsed_ms(now_time, battery_timer) > 1000) {
      battery_timer = now_time;
      Charger.update();
      printf("Charging: %s(0x%02X) | Fault: 0x%02X\n", Charger.charging ? "Yes" : "No", Charger.chrg_stat, Charger.fault); //test
    }
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
  case CMD_VIBRATION_ENABLE:
    Vibration.enable(true);
    break;
  case CMD_VIBRATION_DATA:
    Vibration.update_from_bridge(cmd->payload, cmd->payload_size);
    break;
  case CMD_VIBRATION_DISABLE:
    Vibration.enable(false);
    break;
  default:
    break;
  }
}
// headers
#include "southbridge.hpp"

// hw lib init
btn_matrix BtnMatrix = btn_matrix(PIN_BTN_H1, PIN_BTN_H2, PIN_BTN_H3, PIN_BTN_H4, PIN_BTN_V1, PIN_BTN_V2, PIN_BTN_V3, PIN_BTN_V4);
joystick Joy1 = joystick(PIN_JOY1_X, PIN_JOY1_Y, false, false, PIN_JOY1_BTN);
joystick Joy2 = joystick(PIN_JOY2_X, PIN_JOY2_Y, true,  true,  PIN_JOY2_BTN);
vibrationLRA Lra = vibrationLRA(PIN_LRA_L, PIN_LRA_R);
bq25619 Charger = bq25619(i2c1, PIN_I2C_SDA, PIN_I2C_SCL, PIN_BAT_INT);
mpu6050 Mpu = mpu6050(i2c1, PIN_I2C_SDA, PIN_I2C_SCL, PIN_IMU_INT);
tempNTC TempSensor = tempNTC(PIN_VIN);
ir_pulse_capture_t ir_rx;
ir_tx_t ir_tx;

// middleware lib init
bridgeProtocol Bridge = bridgeProtocol();
gamepad Gamepad = gamepad(&BtnMatrix, &Joy1, &Joy2);
audioSystem Audio = audioSystem();
temperature Temperature = temperature();
vibration Vibration = vibration(&Lra);
imu Imu = imu(&Mpu);
irLink Ir = irLink(&ir_rx, &ir_tx);

void core1_entry(void);
void bridge_do_cmd(const bridge_msg_t* msg);

time_ms_t gamepad_timer;
time_ms_t temperature_timer;
time_ms_t imu_timer;
time_ms_t battery_timer;
time_ms_t ir_timer;

//////// function ////////

int main() {
//  uartLog_init(uart0, 0, 1, 115200);
  stdio_init_all();
  uart_bridge_init(uart1, PIN_BRIDGE_TX, PIN_BRIDGE_RX, 921600);
  Bridge.set_bridge_do_cmd(bridge_do_cmd);

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
  ir_pulse_capture_init(&ir_rx, pio1, PIN_IR_RX); //placeholder for pio
  ir_tx_init(&ir_tx, pio2, PIN_IR_TX); //placeholder for pio
  ir_tx_start(&ir_tx); // test
  Ir.init();

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
    uint8_t temp_payload[PAYLOAD_MAX_SIZE];
    int payload_size = 0;

    Bridge.bridge_handle();
    Bridge.bridge_protocol_execute_cmd();

    if(system_time_elapsed_ms(now_time, gamepad_timer) > 10) {
      gamepad_timer = now_time;
      Gamepad.update();
      payload_size = Gamepad.make_bridge_payload(temp_payload, PAYLOAD_MAX_SIZE);
      if(payload_size > 0) {
        Bridge.bridge_msg_push(CMD_GAMEPAD_DATA, payload_size, temp_payload);
      }
    }
    if(system_time_elapsed_ms(now_time, temperature_timer) > 1000) {
      temperature_timer = now_time;
      Temperature.update();
      payload_size = Temperature.make_bridge_payload(temp_payload, PAYLOAD_MAX_SIZE);
      if(payload_size > 0) {
        Bridge.bridge_msg_push(CMD_TEMPERATURE_DATA, payload_size, temp_payload);
      }

      //printf("voltage : % 3.1fV\n", TempSensor.read()); //test
    }
    if(system_time_elapsed_ms(now_time, imu_timer) > 10) {
      imu_timer = now_time;
      Imu.update();
    }
    if(system_time_elapsed_ms(now_time, battery_timer) > 1000) {
      battery_timer = now_time;
      Charger.update();
      // printf("Charging: %s | Fault: 0x%02X\n", Charger.charging ? "Yes" : "No", Charger.fault); //test
    }
    if(system_time_elapsed_ms(now_time, ir_timer) > 1) {
      ir_timer = now_time;
      Ir.update();
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

void bridge_do_cmd(const bridge_msg_t* msg) {
  enum bridge_cmd command = (enum bridge_cmd)msg->cmd;
  switch (command)
  {
  case CMD_IR_RX_ENABLE:
    Ir.recv_bridge_enable_rx(msg->payload, msg->payload_size);
    break;
  case CMD_IR_RX_DISABLE:
    Ir.enable_rx(false);
    break;
  case CMD_IR_TX_ENABLE:
    Ir.recv_bridge_enable_tx(msg->payload, msg->payload_size);
    break;
  case CMD_IR_TX_DATA:
    Ir.recv_bridge_tx_data(msg->payload, msg->payload_size);
    break;
  case CMD_IR_TX_DISABLE:
    Ir.enable_tx(false);
    break;
  case CMD_IMU_ENABLE:
    Imu.recv_bridge_enable(msg->payload, msg->payload_size);
    break;
  case CMD_IMU_DISABLE:
    Imu.enable_accel_data(false);
    Imu.enable_gyro_data(false);
    break;
  case CMD_AUDIO_ENABLE:
    // Audio.enable();
    break;
  case CMD_AUDIO_PCM_DATA:
    Audio.recv_bridge_data(msg->payload, msg->payload_size);
    break;
  case CMD_AUDIO_DISABLE:
    // Audio.disable();
    break;
  case CMD_VIBRATION_ENABLE:
    Vibration.enable(true);
    break;
  case CMD_VIBRATION_DATA:
    Vibration.recv_bridge_data(msg->payload, msg->payload_size);
    break;
  case CMD_VIBRATION_DISABLE:
    Vibration.enable(false);
    break;
  default:
    break;
  }
}
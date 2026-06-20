#include "gamepad.hpp"
#include "bridge_protocol.hpp"

extern bridgeProtocol Bridge;

gamepad::gamepad(btn_matrix* btn_matrix, joystick* joy_1, joystick* joy_2) {
  btns = btn_matrix;
  joy1 = joy_1;
  joy2 = joy_2;
}

void gamepad::init(void) {
  btns->init();
  if(joy1) joy1->init();
  if(joy2) joy2->init();

  for(int i=0; i<GP_JOYSTICK_CH_NUM; i++) {
    joystick_data[i] = 0;
    joystick_raw[i] = 2048;
  }

  enable = true;
  raw_data_enable = false;
}

void gamepad::update(void) {

  btns->update();
  uint32_t btn_data_raw = btns->get_btn_data();

  // map button data to btn_code enum
  btn_data = 0;
  if(btn_data_raw & (1 << 0)) btn_data |= 1 << BTN_X;
  if(btn_data_raw & (1 << 1)) btn_data |= 1 << BTN_ZR;
  if(btn_data_raw & (1 << 2)) btn_data |= 1 << BTN_UP;
  if(btn_data_raw & (1 << 3)) btn_data |= 1 << BTN_SELECT;
  if(btn_data_raw & (1 << 4)) btn_data |= 1 << BTN_A;
  if(btn_data_raw & (1 << 5)) btn_data |= 1 << BTN_SR;
  if(btn_data_raw & (1 << 6)) btn_data |= 1 << BTN_RIGHT;
  if(btn_data_raw & (1 << 7)) btn_data |= 1 << BTN_ZL;
  if(btn_data_raw & (1 << 8)) btn_data |= 1 << BTN_B;
  if(btn_data_raw & (1 << 9)) btn_data |= 1 << BTN_START;
  if(btn_data_raw & (1 << 10)) btn_data |= 1 << BTN_DOWN;
  if(btn_data_raw & (1 << 11)) btn_data |= 1 << BTN_SUB1;
  if(btn_data_raw & (1 << 12)) btn_data |= 1 << BTN_Y;
  if(btn_data_raw & (1 << 13)) btn_data |= 1 << BTN_SUB2;
  if(btn_data_raw & (1 << 14)) btn_data |= 1 << BTN_LEFT;
  if(btn_data_raw & (1 << 15)) btn_data |= 1 << BTN_SL;

  if(joy1) {
    joy1->update();
    joystick_raw[JOY1_X] = joy1->x_raw;
    joystick_raw[JOY1_Y] = joy1->y_raw;
    btn_data |= joy1->stick_btn_state ? 1 << BTN_S1_CENTER : 0;
  }
  if(joy2) {
    joy2->update();
    joystick_raw[JOY2_X] = joy2->x_raw;
    joystick_raw[JOY2_Y] = joy2->y_raw;
    btn_data |= joy2->stick_btn_state ? 1 << BTN_S2_CENTER : 0;
  }

  // calibration is not implemented yet
  // just convert to -128 ~ 127
  for(int i=0; i<GP_JOYSTICK_CH_NUM; i++) {
    joystick_data[i] = (int16_t)(joystick_raw[i] >> 4) - 128;
  }

  if(enable) {
    send_bridge_data();
  }
  if (raw_data_enable) {
    send_bridge_raw_data();
  }
}

void gamepad::send_bridge_data(void) {
  int payload_size = 7;
  uint8_t payload_buf[PAYLOAD_MAX_SIZE];

  // little endian
  payload_buf[0] = btn_data & 0xFF;
  payload_buf[1] = (btn_data >> 8) & 0xFF;
  payload_buf[2] = (btn_data >> 16) & 0xFF;

  for(int i=0; i<GP_JOYSTICK_CH_NUM; i++) {
    payload_buf[3+i] = joystick_data[i];
  }

  Bridge.send(CMD_GAMEPAD_DATA, payload_size, payload_buf);
}

void gamepad::send_bridge_raw_data(void) {
  int payload_size = 8; // 2 data * 4 axis
  uint8_t payload_buf[PAYLOAD_MAX_SIZE];

  // little endian
  for(int i=0; i<GP_JOYSTICK_CH_NUM; i++) {
    payload_buf[2*i] = joystick_raw[i] & 0xFF;
    payload_buf[2*i+1] = joystick_raw[i] >> 8 & 0xFF;
  }

  Bridge.send(CMD_GAMEPAD_RAW_DATA, payload_size, payload_buf);
}

void gamepad::recv_bridge_enable(const uint8_t* payload, uint8_t payload_size) {
  if(payload_size < 1) return;

  enable = payload[0] & 1 ? true : false;
  raw_data_enable = payload[0] & 2 ? true : false;
}

void gamepad::recv_bridge_cali_data(const uint8_t* payload, uint8_t payload_size) {
  // not implemented
}
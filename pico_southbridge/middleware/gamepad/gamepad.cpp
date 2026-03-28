#include "gamepad.hpp"

gamepad::gamepad(btn_matrix* btn_matrix, joystick* joy_1, joystick* joy_2) {
  btns = btn_matrix;
  joy1 = joy_1;
  joy2 = joy_2;
}

void gamepad::init(void) {
  btns->init();
  if(joy1) joy1->init();
  if(joy2) joy2->init();

  joystick_x[0] = 0;
  joystick_y[0] = 0;
  joystick_x[1] = 0;
  joystick_y[1] = 0;
}

void gamepad::update(void) {

  btns->update();

  if(joy1) {
    joy1->update();
    joystick_x[0] = joy1->x;
    joystick_y[0] = joy1->y;
  }
  if(joy2) {
    joy2->update();
    joystick_x[1] = joy2->x;
    joystick_y[1] = joy2->y;
  }
}

int gamepad::make_bridge_payload(uint8_t* payload_buf, uint max_size) {
  if(max_size < 6) return -1;

  uint32_t btn_data = btns->get_btn_data() ? 1 << BTN_START : 0x0000; //placeholder for START button

  payload_buf[0] = btn_data >> 8 & 0xFF;
  payload_buf[1] = btn_data & 0xFF;
  payload_buf[2] = joystick_x[0]; // joyLx
  payload_buf[3] = joystick_y[0]; // joyLy
  payload_buf[4] = joystick_x[1]; // joyRx
  payload_buf[5] = joystick_y[1]; // joyRy

  return 6;
}
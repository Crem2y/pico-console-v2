#pragma once

#include <stdint.h>
#include "btn_matrix.hpp"
#include "joystick.hpp"

#define GP_BTN_NUM 16

#define GP_JOYSTICK_NUM 2
#define GP_JOYSTICK_MAX 127
#define GP_JOYSTICK_MIN -127

enum btn_code {
  BTN_S1_UP = 0,
  BTN_S1_DOWN,
  BTN_UP,
  BTN_DOWN,
  BTN_LEFT,
  BTN_RIGHT,
  BTN_A,
  BTN_B,
  BTN_X,
  BTN_Y,
  BTN_SL,
  BTN_SR,
  BTN_ZL,
  BTN_ZR,
  BTN_START,
  BTN_SELECT,
  BTN_S1_CENTER,
  BTN_S2_CENTER,
  BTN_NONE = 0xff
};

class gamepad {
  public:
    gamepad(btn_matrix* btn_matrix, joystick* joy_1, joystick* joy_2);

    void init(void);
    void update(void);

    int make_bridge_payload(uint8_t* payload_buf, uint max_size);

    int8_t get_joystick_x(int joystick_num);
    int8_t get_joystick_y(int joystick_num);

  private:
    btn_matrix* btns;
    joystick* joy1;
    joystick* joy2;
    int8_t joystick_x[GP_JOYSTICK_NUM] = {0,};
    int8_t joystick_y[GP_JOYSTICK_NUM] = {0,};
};
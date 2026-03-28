#pragma once

#include <stdint.h>
//#include "pca9554.hpp"
#include "system_time.h"

#define GP_BTN_NUM 24

#define GP_JOYSTICK_NUM 2
#define GP_JOYSTICK_DEADZONE 10
#define GP_JOYSTICK_MAX 127
#define GP_JOYSTICK_MIN -127
#define GP_JOYSTICK_TO_BTN_THRESHOLD 50

enum btn_code {
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

  BTN_S1_UP,
  BTN_S1_DOWN,
  BTN_S1_LEFT,
  BTN_S1_RIGHT,
  BTN_S2_UP,
  BTN_S2_DOWN,
  BTN_S2_LEFT,
  BTN_S2_RIGHT,
  BTN_NONE = 0xff
};

class gamepad {
  public:
    gamepad(void);

    void init(void);
    void update(void);
    void update_from_bridge(uint8_t* data, uint8_t len);

    int is_btn_pressed(enum btn_code btn);
    int is_btn_released(enum btn_code btn);
    time_ms_t get_btn_pressed_duration(enum btn_code btn);
    time_ms_t get_btn_released_duration(enum btn_code btn);

    int8_t get_joystick_x(int joystick_num);
    int8_t get_joystick_y(int joystick_num);

  private:
    time_ms_t current_time_ms;
    uint8_t btn_state[GP_BTN_NUM] = {0,}; // 0 for released, 1 for pressed
    time_ms_t btn_last_pressed_ms[GP_BTN_NUM] = {0,};
    time_ms_t btn_last_released_ms[GP_BTN_NUM] = {0,};
    uint32_t key_data;
    int8_t joystick_x[GP_JOYSTICK_NUM] = {0,};
    int8_t joystick_y[GP_JOYSTICK_NUM] = {0,};
};
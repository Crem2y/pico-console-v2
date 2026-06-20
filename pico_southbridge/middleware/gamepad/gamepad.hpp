#pragma once

#include <stdint.h>
#include "btn_matrix.hpp"
#include "joystick.hpp"

#define GP_BTN_NUM 18

#define GP_JOYSTICK_NUM 2
#define GP_JOYSTICK_CH_NUM (GP_JOYSTICK_NUM * 2)
#define GP_JOYSTICK_MAX 127
#define GP_JOYSTICK_MIN -127

enum btn_code {
  BTN_UP = 0,
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
  BTN_SUB1,
  BTN_SUB2,
  BTN_S1_CENTER,
  BTN_S2_CENTER,
  BTN_NONE = 0xff
};

enum joy_channel {
  JOY1_X = 0,
  JOY1_Y,
  JOY2_X,
  JOY2_Y
};

typedef struct _joy_cali_data
{
  uint16_t center;
  uint16_t deadzone;
  uint16_t min;
  uint16_t max;
} joy_cali_data;


class gamepad {
  public:
    gamepad(btn_matrix* btn_matrix, joystick* joy_1, joystick* joy_2);

    void init(void);
    void update(void);

    int8_t calibrate_joystick(uint16_t raw, const joy_cali_data* cali);

    void send_bridge_data(void);
    void send_bridge_raw_data(void);

    void recv_bridge_enable(const uint8_t* payload, uint8_t payload_size);
    void recv_bridge_cali_data(const uint8_t* payload, uint8_t payload_size);

    private:
    btn_matrix* btns;
    uint32_t btn_data;
    joystick* joy1;
    joystick* joy2;
    int8_t joystick_data[GP_JOYSTICK_CH_NUM];
    uint16_t joystick_raw[GP_JOYSTICK_CH_NUM];
    joy_cali_data cali_data[GP_JOYSTICK_CH_NUM];

    bool enable, raw_data_enable;
};
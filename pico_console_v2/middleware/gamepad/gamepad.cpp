#include "gamepad.hpp"
#include "bridge_protocol.hpp"

extern bridgeProtocol Bridge;

gamepad::gamepad(void) {

}

void gamepad::init(void) {
  for(int i=0; i<GP_BTN_NUM; i++) {
    btn_state[i] = 0;
    btn_last_pressed_ms[i] = 0;
    btn_last_released_ms[i] = 0;
  }

  joystick_x[0] = 0;
  joystick_y[0] = 0;
  joystick_x[1] = 0;
  joystick_y[1] = 0;

  joystick_raw_x[0] = 0;
  joystick_raw_y[0] = 0;
  joystick_raw_x[1] = 0;
  joystick_raw_y[1] = 0;
}

void gamepad::update(void) {
  current_time_ms = get_system_time_ms();

  for(int i=0; i<BTN_S1_UP; i++) {
    uint32_t is_pressed = (key_data & (0x00000001 << i)) ? 1 : 0;
    if(is_pressed && !btn_state[i]) { // button just pressed
      btn_state[i] = 1;
      btn_last_pressed_ms[i] = current_time_ms;
    } else if(!is_pressed && btn_state[i]) { // button just released
      btn_state[i] = 0;
      btn_last_released_ms[i] = current_time_ms;
    }
  }

  // joystick -> btn data update
  // S1 joystick
  if(joystick_x[0] > GP_JOYSTICK_TO_BTN_THRESHOLD) {
    if(!btn_state[BTN_S1_RIGHT]) {
      btn_state[BTN_S1_RIGHT] = 1;
      btn_last_pressed_ms[BTN_S1_RIGHT] = current_time_ms;
    }
  } else if(joystick_x[0] < -GP_JOYSTICK_TO_BTN_THRESHOLD) {
    if(!btn_state[BTN_S1_LEFT]) {
      btn_state[BTN_S1_LEFT] = 1;
      btn_last_pressed_ms[BTN_S1_LEFT] = current_time_ms;
    }
  } else {
    if(btn_state[BTN_S1_RIGHT]) {
      btn_state[BTN_S1_RIGHT] = 0;
      btn_last_released_ms[BTN_S1_RIGHT] = current_time_ms;
    }
    if(btn_state[BTN_S1_LEFT]) {
      btn_state[BTN_S1_LEFT] = 0;
      btn_last_released_ms[BTN_S1_LEFT] = current_time_ms;
    }
  }

  if(joystick_y[0] > GP_JOYSTICK_TO_BTN_THRESHOLD) {
    if(!btn_state[BTN_S1_DOWN]) {
      btn_state[BTN_S1_DOWN] = 1;
      btn_last_pressed_ms[BTN_S1_DOWN] = current_time_ms;
    }
  } else if(joystick_y[0] < -GP_JOYSTICK_TO_BTN_THRESHOLD) {
    if(!btn_state[BTN_S1_UP]) {
      btn_state[BTN_S1_UP] = 1;
      btn_last_pressed_ms[BTN_S1_UP] = current_time_ms;
    }
  } else {
    if(btn_state[BTN_S1_DOWN]) {
      btn_state[BTN_S1_DOWN] = 0;
      btn_last_released_ms[BTN_S1_DOWN] = current_time_ms;
    }
    if(btn_state[BTN_S1_UP]) {
      btn_state[BTN_S1_UP] = 0;
      btn_last_released_ms[BTN_S1_UP] = current_time_ms;
    }
  }

  // S2 joystick
  if(joystick_x[1] > GP_JOYSTICK_TO_BTN_THRESHOLD)
  {
    if(!btn_state[BTN_S2_RIGHT]) {
      btn_state[BTN_S2_RIGHT] = 1;
      btn_last_pressed_ms[BTN_S2_RIGHT] = current_time_ms;
    }
  } else if(joystick_x[1] < -GP_JOYSTICK_TO_BTN_THRESHOLD) {
    if(!btn_state[BTN_S2_LEFT]) {
      btn_state[BTN_S2_LEFT] = 1;
      btn_last_pressed_ms[BTN_S2_LEFT] = current_time_ms;
    }
  } else {
    if(btn_state[BTN_S2_RIGHT]) {
      btn_state[BTN_S2_RIGHT] = 0;
      btn_last_released_ms[BTN_S2_RIGHT] = current_time_ms;
    }
    if(btn_state[BTN_S2_LEFT]) {
      btn_state[BTN_S2_LEFT] = 0;
      btn_last_released_ms[BTN_S2_LEFT] = current_time_ms;
    }
  }

  if(joystick_y[1] > GP_JOYSTICK_TO_BTN_THRESHOLD) {
    if(!btn_state[BTN_S2_DOWN]) {
      btn_state[BTN_S2_DOWN] = 1;
      btn_last_pressed_ms[BTN_S2_DOWN] = current_time_ms;
    }
  } else if(joystick_y[1] < -GP_JOYSTICK_TO_BTN_THRESHOLD) {
    if(!btn_state[BTN_S2_UP]) {
      btn_state[BTN_S2_UP] = 1;
      btn_last_pressed_ms[BTN_S2_UP] = current_time_ms;
    }
  } else {
    if(btn_state[BTN_S2_DOWN]) {
      btn_state[BTN_S2_DOWN] = 0;
      btn_last_released_ms[BTN_S2_DOWN] = current_time_ms;
    }
    if(btn_state[BTN_S2_UP]) {
      btn_state[BTN_S2_UP] = 0;
      btn_last_released_ms[BTN_S2_UP] = current_time_ms;
    }
  }
}

int gamepad::is_btn_pressed(enum btn_code btn) {
  return btn_state[btn];
}

int gamepad::is_btn_released(enum btn_code btn) {
  return !btn_state[btn];
}

time_ms_t gamepad::get_btn_pressed_duration(enum btn_code btn) {
  if(btn_state[btn]) {
    return system_time_elapsed_ms(current_time_ms, btn_last_pressed_ms[btn]);
  } else {
    return 0;
  }
}

time_ms_t gamepad::get_btn_released_duration(enum btn_code btn) {
  if(!btn_state[btn]) {
    return system_time_elapsed_ms(current_time_ms, btn_last_released_ms[btn]);
  } else {
    return 0;
  }
}

int8_t gamepad::get_joystick_x(int joystick_num) {
  if(joystick_num < 0 || joystick_num >= GP_JOYSTICK_NUM) return 0;

  if(joystick_x[joystick_num] > GP_JOYSTICK_MAX) joystick_x[joystick_num] = GP_JOYSTICK_MAX;
  if(joystick_x[joystick_num] < GP_JOYSTICK_MIN) joystick_x[joystick_num] = GP_JOYSTICK_MIN;

  return joystick_x[joystick_num];
}

int8_t gamepad::get_joystick_y(int joystick_num) {
  if(joystick_num < 0 || joystick_num >= GP_JOYSTICK_NUM) return 0;

  if(joystick_y[joystick_num] > GP_JOYSTICK_MAX) joystick_y[joystick_num] = GP_JOYSTICK_MAX;
  if(joystick_y[joystick_num] < GP_JOYSTICK_MIN) joystick_y[joystick_num] = GP_JOYSTICK_MIN;

  return joystick_y[joystick_num];
}

uint16_t gamepad::get_joystick_raw_x(int joystick_num) {
  if(joystick_num < 0 || joystick_num >= GP_JOYSTICK_NUM) return 0;

  return joystick_raw_x[joystick_num];
}

uint16_t gamepad::get_joystick_raw_y(int joystick_num) {
  if(joystick_num < 0 || joystick_num >= GP_JOYSTICK_NUM) return 0;

  return joystick_raw_y[joystick_num];
}

void gamepad::send_bridge_enable(bool enable, bool raw_data_enable) {
  int payload_size = 1;
  uint8_t payload_buf[PAYLOAD_MAX_SIZE];

  payload_buf[0] = (enable ? 1 : 0) | (raw_data_enable ? 2 : 0);

  Bridge.bridge_msg_push(CMD_GAMEPAD_ENABLE, payload_size, payload_buf);
}

void gamepad::recv_bridge_data(const uint8_t* payload, uint8_t payload_size) {
  if(payload_size < 7) return;

  // little endian
  key_data = payload[0] | (payload[1] << 8) | (payload[2] << 16);

  joystick_x[0] = payload[3];
  joystick_y[0] = payload[4];
  joystick_x[1] = payload[5];
  joystick_y[1] = payload[6];
}

void gamepad::recv_bridge_raw_data(const uint8_t* payload, uint8_t payload_size) {
  if(payload_size < 8) return;

  // little endian
  joystick_raw_x[0] = payload[0] | (payload[1] << 8);
  joystick_raw_y[0] = payload[2] | (payload[3] << 8);
  joystick_raw_x[1] = payload[4] | (payload[5] << 8);
  joystick_raw_y[1] = payload[6] | (payload[7] << 8);
}
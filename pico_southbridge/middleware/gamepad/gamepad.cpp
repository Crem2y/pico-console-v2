#include "gamepad.hpp"

gamepad::gamepad(pca9554* key_ptr) {
  key = key_ptr;
}

void gamepad::init(void) {
  key->init();
}

void gamepad::update(void) {
  current_time_ms = get_system_time_ms();

  // btn data update
  key->get_btn_data();

  for(int i=0; i<GP_BTN_NUM; i++) {
    int is_pressed = (key->key_pressed & (0x00000001 << i)) ? 1 : 0;
    if(is_pressed && !btn_state[i]) { // button just pressed
      btn_state[i] = 1;
      btn_last_pressed_ms[i] = current_time_ms;
    } else if(!is_pressed && btn_state[i]) { // button just released
      btn_state[i] = 0;
      btn_last_released_ms[i] = current_time_ms;
    }
  }

  // joystick data update
  // S1 joystick
  if(btn_state[BTN_S1_UP]) {
    joystick_y[0] = GP_JOYSTICK_MAX;
  } else if(btn_state[BTN_S1_DOWN]) {
    joystick_y[0] = GP_JOYSTICK_MIN;
  } else {
    joystick_y[0] = 0;
  }

  if(btn_state[BTN_S1_LEFT]) {
    joystick_x[0] = GP_JOYSTICK_MIN;
  } else if(btn_state[BTN_S1_RIGHT]) {
    joystick_x[0] = GP_JOYSTICK_MAX;
  } else {
    joystick_x[0] = 0;
  }

  // S2 joystick
  if(btn_state[BTN_S2_UP]) {
    joystick_y[1] = GP_JOYSTICK_MAX;
  } else if(btn_state[BTN_S2_DOWN]) {
    joystick_y[1] = GP_JOYSTICK_MIN;
  } else {
    joystick_y[1] = 0;
  }

  if(btn_state[BTN_S2_LEFT]) {
    joystick_x[1] = GP_JOYSTICK_MIN;
  } else if(btn_state[BTN_S2_RIGHT]) {
    joystick_x[1] = GP_JOYSTICK_MAX;
  } else {
    joystick_x[1] = 0;
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
  return joystick_x[joystick_num];
}

int8_t gamepad::get_joystick_y(int joystick_num) {
  if(joystick_num < 0 || joystick_num >= GP_JOYSTICK_NUM) return 0;
  return joystick_y[joystick_num];
}
#include "gamepad.hpp"

gamepad::gamepad(void) {
  //key = key_ptr;
}

void gamepad::init(void) {
  //key->init();
}

void gamepad::update(void) {
  current_time_ms = get_system_time_ms();

  // btn data update
  // key->get_btn_data();

  // for(int i=0; i<GP_BTN_NUM; i++) {
  //   int is_pressed = (key->key_pressed & (0x00000001 << i)) ? 1 : 0;
  //   if(is_pressed && !btn_state[i]) { // button just pressed
  //     btn_state[i] = 1;
  //     btn_last_pressed_ms[i] = current_time_ms;
  //   } else if(!is_pressed && btn_state[i]) { // button just released
  //     btn_state[i] = 0;
  //     btn_last_released_ms[i] = current_time_ms;
  //   }
  // }

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
}

void gamepad::force_update(enum btn_code btn, int pressed) {
  current_time_ms = get_system_time_ms();

  if(pressed && !btn_state[btn]) { // button just pressed
    btn_state[btn] = 1;
    btn_last_pressed_ms[btn] = current_time_ms;
  } else if(!pressed && btn_state[btn]) { // button just released
    btn_state[btn] = 0;
    btn_last_released_ms[btn] = current_time_ms;
  }
}

void gamepad::force_update_stick(int joystick_num, int8_t x, int8_t y) {
  if(joystick_num < 0 || joystick_num >= GP_JOYSTICK_NUM) return;
  joystick_x[joystick_num] = x;
  joystick_y[joystick_num] = y;
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
#include "touchscreen.hpp"

touchscreen::touchscreen(xpt2046* touch_controller) {
  _touch_controller = touch_controller;
}

void touchscreen::init(void) {
  _touch_controller->init();

  _x_min = 0;
  _x_max = XPT2046_MAX_X;
  _y_min = 0;
  _y_max = XPT2046_MAX_Y;

  _screen_width = 100;
  _screen_height = 100;
  _rotation = 0;
  _swap_xy = false;
  _invert_x = false;
  _invert_y = false;

  touch_state = false;
  touch_point = {.x = -1, .y = -1};
  last_touched_point = {.x = -1, .y = -1};
  last_released_point = {.x = -1, .y = -1};
}

void touchscreen::update(void) {
  // get touch point data
  _touch_controller->get_touch_data();

  touch_data_t raw_touch_data = _touch_controller->touch_data;
  if(_invert_x) {
    raw_touch_data.x = _x_max - raw_touch_data.x;
  }
  if(_invert_y) {
    raw_touch_data.y = _y_max - raw_touch_data.y;
  }
  if(_swap_xy) {
    uint16_t temp = raw_touch_data.x;
    raw_touch_data.x = raw_touch_data.y;
    raw_touch_data.y = temp;
  }

  touch_point.x = ((raw_touch_data.x - _x_min) * _screen_width) / (_x_max - _x_min);
  touch_point.y = ((raw_touch_data.y - _y_min) * _screen_height) / (_y_max - _y_min);

  // get touch state
  if(touch_state) {
    if(raw_touch_data.z1 < TS_RELEASE_THRESHOLD) { // just released
      touch_state = false;
      last_released_point = touch_point;
    }
  } else {
    if(raw_touch_data.z1 > TS_TOUCH_THRESHOLD) { // just touched
      touch_state = true;
      last_touched_point = touch_point;
    }
  }
}

void touchscreen::set_calibration(uint16_t x_min, uint16_t x_max, uint16_t y_min, uint16_t y_max) {
  _x_min = x_min;
  _x_max = x_max;
  _y_min = y_min;
  _y_max = y_max;
}

void touchscreen::set_screen_size(uint16_t width, uint16_t height) {
  _screen_width = width;
  _screen_height = height;
}

void touchscreen::set_screen_config(bool swap_xy, bool invert_x, bool invert_y) {
  _swap_xy = swap_xy;
  _invert_x = invert_x;
  _invert_y = invert_y;
}

void touchscreen::set_rotation(uint8_t rotation) {
  _rotation = rotation % 4;
  switch(_rotation) {
    case 0:
      _swap_xy = true;
      _invert_x = false;
      _invert_y = false;
      break;
    case 1:
      _swap_xy = false;
      _invert_x = false;
      _invert_y = true;
      break;
    case 2:
      _swap_xy = true;
      _invert_x = true;
      _invert_y = true;
      break;
    case 3:
      _swap_xy = false;
      _invert_x = true;
      _invert_y = false;
      break;
  }
}

touch_point_t touchscreen::get_touch_point(void) {
  touch_point_t point;

  if(!touch_state) {
    point = {.x = -1, .y = -1};
  } else {
    point = touch_point;
  }

  return point;
}
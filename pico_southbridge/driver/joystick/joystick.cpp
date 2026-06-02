#include "joystick.hpp"

joystick::joystick(int stick_x_pin, int stick_y_pin, bool invert_x, bool invert_y, int stick_btn_pin) {
  _stick_btn_pin = stick_btn_pin;
  _stick_x_pin = stick_x_pin;
  _stick_y_pin = stick_y_pin;
  _ch_x = _stick_x_pin - ADC_BASE_PIN;
  _ch_y = _stick_y_pin - ADC_BASE_PIN;
  _invert_x = invert_x;
  _invert_y = invert_y;
}

void joystick::init(void) {
  adc_init();
  adc_gpio_init(_stick_x_pin);
  adc_gpio_init(_stick_y_pin);
  if(_stick_btn_pin > -1) {
    gpio_init(_stick_btn_pin);
    gpio_set_dir(_stick_btn_pin, GPIO_IN);
    gpio_pull_up(_stick_btn_pin);
  }

  stick_btn_state = 0;
  x_raw = 0;
  y_raw = 0;
}

void joystick::update(void) {
  if(_stick_btn_pin > -1) {
    stick_btn_state = !gpio_get(_stick_btn_pin);
  }
  adc_select_input(_ch_x);
  x_raw = adc_read();
  adc_select_input(_ch_y);
  y_raw = adc_read();

  if(_invert_x) x_raw = 4095 - x_raw;
  if(_invert_y) y_raw = 4095 - y_raw;
}
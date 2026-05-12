#include "joystick.hpp"

joystick::joystick(int stick_x_pin, int stick_y_pin, bool invert_x, bool invert_y) {
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
}

void joystick::update(void) {
  adc_select_input(_ch_x);
  uint16_t stick_x_raw = adc_read();
  adc_select_input(_ch_y);
  uint16_t stick_y_raw = adc_read();

  if(_invert_x) stick_x_raw = 4095 - stick_x_raw;
  x = (int16_t)(stick_x_raw >> 4) - 128; // convert to -128 ~ 127

  if(_invert_y) stick_y_raw = 4095 - stick_y_raw;
  y = (int16_t)(stick_y_raw >> 4) - 128; // convert to -128 ~ 127
}
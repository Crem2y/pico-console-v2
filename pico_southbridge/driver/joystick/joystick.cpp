#include "joystick.hpp"

joystick::joystick(int stick_x_pin, int stick_y_pin, uint ch_x, uint ch_y) {
  _stick_x_pin = stick_x_pin;
  _stick_y_pin = stick_y_pin;
  _ch_x = ch_x;
  _ch_y = ch_y;
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

  x = (int16_t)(stick_x_raw >> 4) - 128; // convert to -128 ~ 127
  y = (int16_t)(stick_y_raw >> 4) - 128; // convert to -128 ~ 127
}
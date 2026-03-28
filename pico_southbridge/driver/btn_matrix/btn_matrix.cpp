#include "btn_matrix.hpp"

btn_matrix::btn_matrix(int btn_pin) {
  _btn_pin = btn_pin;
}

void btn_matrix::init(void) {
  gpio_init(_btn_pin);
  gpio_set_dir(_btn_pin, GPIO_IN);
  gpio_pull_up(_btn_pin);
}

void btn_matrix::update(void) {
  btn_data = gpio_get(_btn_pin) ? 0 : 1;
}

int32_t btn_matrix::get_btn_data(void) {
  return btn_data;
}
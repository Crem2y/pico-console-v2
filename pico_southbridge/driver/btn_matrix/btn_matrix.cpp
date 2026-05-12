#include "btn_matrix.hpp"

btn_matrix::btn_matrix(int h1_pin, int h2_pin, int h3_pin, int h4_pin, int v1_pin, int v2_pin, int v3_pin, int v4_pin) {
  _h1_pin = h1_pin;
  _h2_pin = h2_pin;
  _h3_pin = h3_pin;
  _h4_pin = h4_pin;
  _v1_pin = v1_pin;
  _v2_pin = v2_pin;
  _v3_pin = v3_pin;
  _v4_pin = v4_pin;
}

void btn_matrix::init(void) {
  gpio_init(_h1_pin);
  gpio_set_dir(_h1_pin, GPIO_IN);
  gpio_pull_down(_h1_pin);

  gpio_init(_h2_pin);
  gpio_set_dir(_h2_pin, GPIO_IN);
  gpio_pull_down(_h2_pin);

  gpio_init(_h3_pin);
  gpio_set_dir(_h3_pin, GPIO_IN);
  gpio_pull_down(_h3_pin);

  gpio_init(_h4_pin);
  gpio_set_dir(_h4_pin, GPIO_IN);
  gpio_pull_down(_h4_pin);

  gpio_init(_v1_pin);
  gpio_set_dir(_v1_pin, GPIO_OUT);
  gpio_put(_v1_pin, 0);

  gpio_init(_v2_pin);
  gpio_set_dir(_v2_pin, GPIO_OUT);
  gpio_put(_v2_pin, 0);

  gpio_init(_v3_pin);
  gpio_set_dir(_v3_pin, GPIO_OUT);
  gpio_put(_v3_pin, 0);

  gpio_init(_v4_pin);
  gpio_set_dir(_v4_pin, GPIO_OUT);
  gpio_put(_v4_pin, 0);

  btn_data = 0;
}

void btn_matrix::update(void) {
  btn_data = 0;

  gpio_put(_v1_pin, 1);
  btn_data |= gpio_get(_h1_pin) ? (1 << 0) : 0;
  btn_data |= gpio_get(_h2_pin) ? (1 << 1) : 0;
  btn_data |= gpio_get(_h3_pin) ? (1 << 2) : 0;
  btn_data |= gpio_get(_h4_pin) ? (1 << 3) : 0;
  gpio_put(_v1_pin, 0);

  sleep_us(10); // delay for signal stabilization

  gpio_put(_v2_pin, 1);
  btn_data |= gpio_get(_h1_pin) ? (1 << 4) : 0;
  btn_data |= gpio_get(_h2_pin) ? (1 << 5) : 0;
  btn_data |= gpio_get(_h3_pin) ? (1 << 6) : 0;
  btn_data |= gpio_get(_h4_pin) ? (1 << 7) : 0;
  gpio_put(_v2_pin, 0);

  sleep_us(10);

  gpio_put(_v3_pin, 1);
  btn_data |= gpio_get(_h1_pin) ? (1 << 8) : 0;
  btn_data |= gpio_get(_h2_pin) ? (1 << 9) : 0;
  btn_data |= gpio_get(_h3_pin) ? (1 << 10) : 0;
  btn_data |= gpio_get(_h4_pin) ? (1 << 11) : 0;
  gpio_put(_v3_pin, 0);

  sleep_us(10);

  gpio_put(_v4_pin, 1);
  btn_data |= gpio_get(_h1_pin) ? (1 << 12) : 0;
  btn_data |= gpio_get(_h2_pin) ? (1 << 13) : 0;
  btn_data |= gpio_get(_h3_pin) ? (1 << 14) : 0;
  btn_data |= gpio_get(_h4_pin) ? (1 << 15) : 0;
  gpio_put(_v4_pin, 0);
}

uint32_t btn_matrix::get_btn_data(void) {
  return btn_data;
}
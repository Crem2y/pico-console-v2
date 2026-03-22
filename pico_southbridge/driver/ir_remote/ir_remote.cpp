#include "ir_remote.hpp"

irRemote::irRemote(int ir_pin) {
  _ir_pin = ir_pin;
}

void irRemote::init(void) {
  gpio_init(_ir_pin);
  gpio_set_dir(_ir_pin, GPIO_OUT);
  gpio_put(_ir_pin, 0);


  gpio_set_function(_ir_pin, GPIO_FUNC_PWM);
  slice_num = pwm_gpio_to_slice_num(_ir_pin);

  ir_pwm_ch = (_ir_pin % 2) ? PWM_CHAN_B : PWM_CHAN_A;

  pwm_set_chan_level(slice_num, ir_pwm_ch, 0);
  pwm_set_clkdiv(slice_num, (SYS_CLK_KHZ * 1000) / 1000000);
  ir_pwm_top = 1000000 / 38000;
  pwm_set_wrap(slice_num, ir_pwm_top);
  pwm_set_enabled(slice_num, true);
}

void irRemote::manual_on(void) {
  //gpio_put(_ir_pin, 1);
  pwm_set_chan_level(slice_num, ir_pwm_ch, ir_pwm_top);
}

void irRemote::manual_off(void) {
  //gpio_put(_ir_pin, 0);
  pwm_set_chan_level(slice_num, ir_pwm_ch, 0);
}

inline void irRemote::send_nec_bit(uint8_t data) {
  if (data) { // if 1, 562.5us high and 1687.5us low
    //gpio_put(_ir_pin, 1);
    pwm_set_chan_level(slice_num, ir_pwm_ch, ir_pwm_top/2);
    sleep_us(562);
    //gpio_put(_ir_pin, 0);
    pwm_set_chan_level(slice_num, ir_pwm_ch, 0);
    sleep_us(1688);
  } else {  // if 0, 562.5us high and 562.5us low
    //gpio_put(_ir_pin, 1);
    pwm_set_chan_level(slice_num, ir_pwm_ch, ir_pwm_top/2);
    sleep_us(562);
    //gpio_put(_ir_pin, 0);
    pwm_set_chan_level(slice_num, ir_pwm_ch, 0);
    sleep_us(563);
  }
}

inline void irRemote::send_nec_byte(uint8_t data) {
  uint8_t temp = 0x01;
  for(int i=0; i<8; i++) {
    send_nec_bit(data & temp);
    temp <<= 1;
  }
}

void irRemote::send_nec_format(bool short_leader, uint8_t custom1, uint8_t custom2, uint8_t data) {
  if(short_leader) {
    //gpio_put(_ir_pin, 1);
    pwm_set_chan_level(slice_num, ir_pwm_ch, ir_pwm_top/2);
    sleep_us(9000);
  } else {
    //gpio_put(_ir_pin, 1);
    pwm_set_chan_level(slice_num, ir_pwm_ch, ir_pwm_top/2);
    sleep_us(4500);
  }
  //gpio_put(_ir_pin, 0);
  pwm_set_chan_level(slice_num, ir_pwm_ch, 0);
  sleep_us(4500);

  send_nec_byte(custom1);
  send_nec_byte(custom2);
  send_nec_byte(data);
  send_nec_byte(~data);
  send_nec_bit(1);
}


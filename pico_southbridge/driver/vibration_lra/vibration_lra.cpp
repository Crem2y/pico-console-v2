#include "vibration_lra.hpp"

static vibrationLRA* lra;

static bool output_level_l = false;
static bool output_level_r = false;

int64_t lra_l_timer_callback(alarm_id_t id, void *user_data) {
  if(lra->lra_dulation_us[0] > 0) {
    output_level_l = !output_level_l;
    if(output_level_l) {
      pwm_set_chan_level(lra->slice_num[0], lra->lra_pwm_ch[0], lra->lra_power[0]); // pwm on
    } else {
      pwm_set_chan_level(lra->slice_num[0], lra->lra_pwm_ch[0], 0); // pwm off
    }
    add_alarm_in_us(lra->lra_dulation_us[0], lra_l_timer_callback, NULL, true);
  } else {
    output_level_l = false;
    pwm_set_chan_level(lra->slice_num[0], lra->lra_pwm_ch[0], 0); // pwm off
    add_alarm_in_us(1000, lra_l_timer_callback, NULL, true); // check every 1ms if the duration is set again
  }
  return 0;
}

int64_t lra_r_timer_callback(alarm_id_t id, void *user_data) {
  if(lra->lra_dulation_us[1] > 0) {
    output_level_r = !output_level_r;
    if(output_level_r) {
      pwm_set_chan_level(lra->slice_num[1], lra->lra_pwm_ch[1], lra->lra_power[1]); // pwm on
    } else {
      pwm_set_chan_level(lra->slice_num[1], lra->lra_pwm_ch[1], 0); // pwm off
    }
    add_alarm_in_us(lra->lra_dulation_us[1], lra_r_timer_callback, NULL, true);
  } else {
    output_level_r = false;
    pwm_set_chan_level(lra->slice_num[1], lra->lra_pwm_ch[1], 0); // pwm off
    add_alarm_in_us(1000, lra_r_timer_callback, NULL, true); // check every 1ms if the duration is set again
  }
  return 0;
}

vibrationLRA::vibrationLRA(int lra_l_pin, int lra_r_pin) {
  lra_pin[0] = lra_l_pin;
  lra_pin[1] = lra_r_pin;
}

void vibrationLRA::init(void) {
  lra = this;

  for (int i = 0; i < LRA_NUM; i++) {
    slice_num[i] = pwm_gpio_to_slice_num(lra_pin[i]);
    lra_pwm_ch[i] = pwm_gpio_to_channel(lra_pin[i]);

    gpio_set_function(lra_pin[i], GPIO_FUNC_PWM);

    pwm_set_chan_level(slice_num[i], lra_pwm_ch[i], 1000);
    pwm_set_clkdiv(slice_num[i], SYS_CLK_MHZ / 1000000); // 1 tick = 1 us
    pwm_set_wrap(slice_num[i], 1000000 / 65535); // 65.535 kHz
    pwm_set_enabled(slice_num[i], false);
    lra_power[i] = 0;
    lra_dulation_us[i] = 0;
    lra_enabled = false;
  }
  add_alarm_in_us(1000, lra_l_timer_callback, NULL, true);
  add_alarm_in_us(1000, lra_r_timer_callback, NULL, true);
}

void vibrationLRA::set_power(uint32_t num, uint8_t power) {
  if (num >= LRA_NUM) return;
  if (power > LRA_PWM_MAX) power = LRA_PWM_MAX;
  lra_power[num] = power;
}

void vibrationLRA::set_freq(uint32_t num, uint32_t freq) {
  if (num >= LRA_NUM) return;
  if (freq > LRA_FREQ_MAX) freq = LRA_FREQ_MAX;

  if (freq == 0) {
    lra_dulation_us[num] = 0;
    return;
  }
  lra_dulation_us[num] = 1000000 / freq / 2;
}

void vibrationLRA::enable(void) {
  lra_enabled = true;
  for (int i = 0; i < LRA_NUM; i++) {
    pwm_set_enabled(slice_num[i], true);
  }
}

void vibrationLRA::disable(void) {
  lra_enabled = false;
  for (int i = 0; i < LRA_NUM; i++) {
    pwm_set_enabled(slice_num[i], false);
  }
}

void vibrationLRA::update_from_bridge(uint8_t* data, uint8_t len) {
  if (len < 6) return; // Not enough data

  uint32_t l_freq = data[0] | (data[1] << 8);
  uint8_t l_power = data[2];
  uint32_t r_freq = data[3] | (data[4] << 8);
  uint8_t r_power = data[5];

  set_freq(0, l_freq);
  set_power(0, l_power);
  set_freq(1, r_freq);
  set_power(1, r_power);
}
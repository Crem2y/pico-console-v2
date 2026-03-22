#include "led_control.hpp"

ledControl::ledControl(ledStatus* led_ptr) {
  led = led_ptr;
}

void ledControl::init(void) {
  built_in_led_init();
  led->init();

  for(int i=0; i<LED_CTRL_NUM; i++) {
    led_controls[i].mode = LED_OFF;
    led_controls[i].brightness = LED_CTRL_BRIGHTNESS_MIN;
    led_controls[i].current_brightness = LED_CTRL_BRIGHTNESS_MIN;
    led_controls[i].blink_interval_ms = 0;
    led_controls[i].blink_last_update_ms = 0;
    led_controls[i].breathing_step = 0;
  }
  update();
}

void ledControl::update(void) {
  current_time_ms = get_system_time_ms();

  for(int i=0; i<LED_CTRL_NUM; i++) {
    update_led((enum led_ctrl_name)i);
  }
}

void ledControl::update_led(enum led_ctrl_name led_name) {
  uint32_t num = led_name;

  if(led_name == LED_CONTROL_BUILT_IN) {
    switch (led_controls[num].mode) {
      case LED_OFF:
        built_in_led_set(0);
        led_controls[num].current_brightness = LED_CTRL_BRIGHTNESS_MIN;
        break;
      case LED_ON:
        built_in_led_set(1);
        led_controls[num].current_brightness = LED_CTRL_BRIGHTNESS_MAX;
        break;
      case LED_BLINK_ONCE:
        if(system_time_elapsed_ms(current_time_ms, led_controls[num].blink_last_update_ms) >= led_controls[num].blink_interval_ms) {
          if (led_controls[num].current_brightness == 0) {
            built_in_led_set(1);
            led_controls[num].current_brightness = LED_CTRL_BRIGHTNESS_MAX;
          } else {
            built_in_led_set(0);
            led_controls[num].mode = LED_OFF;
            led_controls[num].current_brightness = LED_CTRL_BRIGHTNESS_MIN;
          }
          led_controls[num].blink_last_update_ms = current_time_ms;
        }
        break;
      case LED_BLINK_REPEAT:
        if(system_time_elapsed_ms(current_time_ms, led_controls[num].blink_last_update_ms) >= led_controls[num].blink_interval_ms) {
          if (led_controls[num].current_brightness == 0) {
            built_in_led_set(1);
            led_controls[num].current_brightness = LED_CTRL_BRIGHTNESS_MAX;
          } else {
            built_in_led_set(0);
            led_controls[num].current_brightness = LED_CTRL_BRIGHTNESS_MIN;
          }
          led_controls[num].blink_last_update_ms = current_time_ms;
        }
        break;
      default:
        break;
    }
    return;
  } else {
    switch (led_controls[num].mode) {
    case LED_OFF:
      led->set_bright(num, LED_PWM_MIN);
      led_controls[num].current_brightness = LED_CTRL_BRIGHTNESS_MIN;
      break;
    case LED_ON:
    {
      uint16_t led_pwm = (uint16_t)(led_controls[num].brightness * ((float)LED_PWM_MAX/LED_CTRL_BRIGHTNESS_MAX));
      led->set_bright(num, led_pwm);
      led_controls[num].current_brightness = led_controls[num].brightness;
      break;
    }
    case LED_BLINK_ONCE:
      if(system_time_elapsed_ms(current_time_ms, led_controls[num].blink_last_update_ms) >= led_controls[num].blink_interval_ms) {
          if (led_controls[num].current_brightness == 0) {
            uint16_t led_pwm = (uint16_t)(led_controls[num].brightness * ((float)LED_PWM_MAX/LED_CTRL_BRIGHTNESS_MAX));
            led->set_bright(num, led_pwm);
            led_controls[num].current_brightness = led_controls[num].brightness;
          } else {
            led->set_bright(num, LED_PWM_MIN);
            led_controls[num].mode = LED_OFF;
            led_controls[num].current_brightness = LED_CTRL_BRIGHTNESS_MIN;
          }
          led_controls[num].blink_last_update_ms = current_time_ms;
        }
      break;
    case LED_BLINK_REPEAT:
      if(system_time_elapsed_ms(current_time_ms, led_controls[num].blink_last_update_ms) >= led_controls[num].blink_interval_ms) {
          if (led_controls[num].current_brightness == 0) {
            uint16_t led_pwm = (uint16_t)(led_controls[num].brightness * ((float)LED_PWM_MAX/LED_CTRL_BRIGHTNESS_MAX));
            led->set_bright(num, led_pwm);
            led_controls[num].current_brightness = led_controls[num].brightness;
          } else {
            led->set_bright(num, LED_PWM_MIN);
            led_controls[num].current_brightness = LED_CTRL_BRIGHTNESS_MIN;
          }
          led_controls[num].blink_last_update_ms = current_time_ms;
        }
      break;
    case LED_BRIGHTER:
      if(led_controls[num].current_brightness < LED_CTRL_BRIGHTNESS_MAX) {
        uint16_t led_pwm = (uint16_t)(led_controls[num].current_brightness * ((float)LED_PWM_MAX/LED_CTRL_BRIGHTNESS_MAX));
        led->set_bright(num, led_pwm);
        if (led_controls[num].current_brightness < LED_CTRL_BRIGHTNESS_MAX - 10) {
          led_controls[num].current_brightness += 10;
        } else {
          led_controls[num].current_brightness = LED_CTRL_BRIGHTNESS_MAX;
          led_controls[num].mode = LED_ON;
        }
      }
      break;
    case LED_DARKER:
      if(led_controls[num].current_brightness > LED_CTRL_BRIGHTNESS_MIN) {
        uint16_t led_pwm = (uint16_t)(led_controls[num].current_brightness * ((float)LED_PWM_MAX/LED_CTRL_BRIGHTNESS_MAX));
        led->set_bright(num, led_pwm);
        if (led_controls[num].current_brightness > 10) {
          led_controls[num].current_brightness -= 10;
        } else {
          led_controls[num].current_brightness = 0;
          led_controls[num].mode = LED_OFF;
        }
      }
      break;
    case LED_BREATHING_ONCE:
      if(system_time_elapsed_ms(current_time_ms, led_controls[num].breathing_last_update_ms) >= 20) {
        uint16_t led_pwm = (uint16_t)(led_controls[num].current_brightness * ((float)LED_PWM_MAX/LED_CTRL_BRIGHTNESS_MAX));
        led->set_bright(num, led_pwm);
        if (led_controls[num].breathing_direction) {
          if (led_controls[num].current_brightness < LED_CTRL_BRIGHTNESS_MAX - led_controls[num].breathing_step) {
            led_controls[num].current_brightness += led_controls[num].breathing_step;
          } else {
            led_controls[num].current_brightness = LED_CTRL_BRIGHTNESS_MAX;
            led_controls[num].breathing_direction = 0;
          }
        } else {
          if (led_controls[num].current_brightness > led_controls[num].breathing_step) {
            led_controls[num].current_brightness -= led_controls[num].breathing_step;
          } else {
            led_controls[num].current_brightness = LED_CTRL_BRIGHTNESS_MIN;
            led_controls[num].mode = LED_OFF;
          }
        }
        led_controls[num].breathing_last_update_ms = current_time_ms;
      }
      break;
    case LED_BREATHING_REPEAT:
      if(system_time_elapsed_ms(current_time_ms, led_controls[num].breathing_last_update_ms) >= 20) {
        uint16_t led_pwm = (uint16_t)(led_controls[num].current_brightness * ((float)LED_PWM_MAX/LED_CTRL_BRIGHTNESS_MAX));
        led->set_bright(num, led_pwm);
        if (led_controls[num].breathing_direction) {
          if (led_controls[num].current_brightness < LED_CTRL_BRIGHTNESS_MAX - led_controls[num].breathing_step) {
            led_controls[num].current_brightness += led_controls[num].breathing_step;
          } else {
            led_controls[num].current_brightness = LED_CTRL_BRIGHTNESS_MAX;
            led_controls[num].breathing_direction = 0;
          }
        } else {
          if (led_controls[num].current_brightness > led_controls[num].breathing_step) {
            led_controls[num].current_brightness -= led_controls[num].breathing_step;
          } else {
            led_controls[num].current_brightness = LED_CTRL_BRIGHTNESS_MIN;
            led_controls[num].breathing_direction = 1;
          }
        }
        led_controls[num].breathing_last_update_ms = current_time_ms;
      }
      break;
    default:
      break;
    }
  }
}

void ledControl::set_mode(enum led_ctrl_name led_name, enum led_ctrl_mode mode) {
  uint32_t num = led_name;
  led_controls[num].mode = mode;
  led_controls[num].blink_last_update_ms = current_time_ms;
  led_controls[num].breathing_last_update_ms = current_time_ms;
  led_controls[num].breathing_direction = 1;
}

void ledControl::set_brightness(enum led_ctrl_name led_name, uint8_t brightness) {
  uint32_t num = led_name;
  led_controls[num].brightness = brightness;
}

void ledControl::set_blink_interval(enum led_ctrl_name led_name, uint32_t interval_ms) {
  uint32_t num = led_name;
  led_controls[num].blink_interval_ms = interval_ms;
}

void ledControl::set_breathing_step(enum led_ctrl_name led_name, uint8_t step) {
  uint32_t num = led_name;
  led_controls[num].breathing_step = step;
}
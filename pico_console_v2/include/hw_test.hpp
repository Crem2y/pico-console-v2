#pragma once

#include <stdio.h>

// pico libraries
#include "pico/stdlib.h"
#include "pico/multicore.h"

// drivers
#include "uart_log.h"
#include "uart_bridge.h"
#include "led_builtin.h"
#include "led_status.hpp"
#include "li_battery.hpp"
#include "ili9488_40_hstx.hpp"
#include "system_time.h"
#include "temp_builtin.h"
#include "xpt2046.hpp"

// middlewares
#include "gamepad.hpp"
#include "audio_system.hpp"
#include "led_control.hpp"
#include "temperature.hpp"

enum menu_main {
  MAIN_BTN_TEST,
  MAIN_LED_TEST,
  MAIN_LCD_TEST,
  MAIN_AUDIO_TEST,
  MAIN_BAT_TEST,
  MAIN_TEMP_TEST,
  MAIN_IR_TEST,
  MAIN_SD_TEST,
};

void menu_btn_test();
void menu_led_test();
void menu_lcd_test();
void menu_audio_test();
void menu_bat_test();
void menu_temp_test();
void menu_ir_test();
void menu_sd_test();
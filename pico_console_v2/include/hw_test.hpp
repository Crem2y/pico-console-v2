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
#include "touchscreen.hpp"
#include "vibration.hpp"
#include "imu.hpp"

enum menu_main {
  MAIN_BTN_TEST,
  MAIN_JOYSTICK_TEST,
  MAIN_LED_TEST,
  MAIN_LCD_TEST,
  MAIN_TOUCH_TEST,
  MAIN_AUDIO_TEST,
  MAIN_VIBRATION_TEST,
  MAIN_BAT_TEST,
  MAIN_TEMP_TEST,
  MAIN_IR_TEST,
  MAIN_IMU_TEST,
  MAIN_SD_TEST,
};

void menu_btn_test();
void menu_joystick_test();
void menu_led_test();
void menu_lcd_test();
void menu_touch_test();
void menu_audio_test();
void menu_vibration_test();
void menu_bat_test();
void menu_temp_test();
void menu_ir_test();
void menu_imu_test();
void menu_sd_test();


// pin definitions
#define PIN_LOG_TX      0
#define PIN_LOG_RX      1
#define PIN_SDIO_CLK    2
#define PIN_SDIO_CMD    3
#define PIN_SDIO_D0     4
#define PIN_SDIO_D1     5
#define PIN_SDIO_D2     6
#define PIN_SDIO_D3     7
#define PIN_PSRAM_CS    8
#define PIN_SDIO_DET    9
#define PIN_BUILTIN_LED 10
// reserved 11
#define PIN_BRIDGE_TX   12
#define PIN_BRIDGE_RX   13
#define PIN_DP_CS       14
#define PIN_DP_DC       15
#define PIN_DP_SCK      16
#define PIN_DP_MOSI     17
#define PIN_DP_RST      18
#define PIN_DP_BL       19
#define PIN_TOUCH_MISO  20
#define PIN_TOUCH_CS    21
#define PIN_TOUCH_SCK   22
#define PIN_TOUCH_MOSI  23
#define PIN_TOUCH_IRQ   24
#define PIN_SB_RESET    25
#define PIN_LED_WL_1    26
#define PIN_LED_WL_2    27
#define PIN_LED_WL_3    28
#define PIN_LED_WL_4    29
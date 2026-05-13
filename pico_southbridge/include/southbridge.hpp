#pragma once

#include <stdio.h>

// pico libraries
#include "pico/stdlib.h"
#include "pico/multicore.h"

// drivers
// #include "uart_log.h"
#include "uart_bridge.h"
#include "i2s_pcm.h"
#include "btn_matrix.hpp"
#include "joystick.hpp"
// #include "li_battery.hpp"
// #include "ir_remote.hpp"
#include "system_time.h"
#include "temp_builtin.h"
#include "vibration_lra.hpp"
#include "bq25619.hpp"
#include "mpu6050.hpp"

// middlewares
#include "audio_system.hpp"
#include "gamepad.hpp"
#include "temperature.hpp"
#include "vibration.hpp"


// pin definitions
#define PIN_LOG_TX      0
#define PIN_LOG_RX      1
#define PIN_IR_TX       2
#define PIN_IR_RX       3
#define PIN_BRIDGE_TX   4
#define PIN_BRIDGE_RX   5
// reserved 6-7
#define PIN_LRA_L       8
#define PIN_LRA_R       9
// reserved 10-11
// reserved 12-19 (HSTX)
#define PIN_QWIIC_SDA   20
#define PIN_QWIIC_SCL   21
#define PIN_I2C_SDA     22
#define PIN_I2C_SCL     23
#define PIN_IMU_INT     24
#define PIN_BAT_INT     25
#define PIN_JOY1_BTN    26
#define PIN_JOY2_BTN    27
#define PIN_BTN_V4      28
#define PIN_BTN_V1      29
#define PIN_BTN_H2      30
#define PIN_BTN_V3      31
#define PIN_BTN_H4      32
#define PIN_BTN_H1      33
#define PIN_BTN_V2      34
#define PIN_BTN_H3      35
#define PIN_I2S_DATA    36
#define PIN_I2S_SCK     37
#define PIN_I2S_LRCLK   38
#define PIN_DAC_MUTE    39
#define PIN_JOY1_X      40
#define PIN_JOY1_Y      41
#define PIN_JOY2_X      42
#define PIN_JOY2_Y      43
#define PIN_VIN         44
// reserved 45-47
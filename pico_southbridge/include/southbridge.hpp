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
#include "li_battery.hpp"
#include "ir_rx.h"
#include "ir_tx.h"
#include "system_time.h"
#include "temp_builtin.h"
#include "temp_ntc.hpp"
#include "vibration_lra.hpp"
#include "bq25619.hpp"
#include "mpu6050.hpp"

// middlewares
#include "bridge_protocol.hpp"
#include "charger.hpp"
#include "audio_system.hpp"
#include "gamepad.hpp"
#include "temperature.hpp"
#include "vibration.hpp"
#include "imu.hpp"
#include "ir_link.hpp"
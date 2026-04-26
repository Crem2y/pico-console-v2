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

// middlewares
#include "audio_system.hpp"
#include "gamepad.hpp"
#include "temperature.hpp"
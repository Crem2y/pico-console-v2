#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "hardware/pio.h"

typedef struct {
  PIO pio;
  uint sm;
  uint offset;
  uint pin;
  bool current_level;
  bool running;
} ir_pulse_capture_t;

typedef struct {
  bool level;
  uint32_t duration_us;
} ir_pulse_t;

#ifdef __cplusplus
extern "C" {
#endif

bool ir_pulse_capture_init(ir_pulse_capture_t *cap, PIO pio, uint pin);
void ir_pulse_capture_start(ir_pulse_capture_t *cap);
void ir_pulse_capture_stop(ir_pulse_capture_t *cap);
bool ir_pulse_capture_available(ir_pulse_capture_t *cap);
bool ir_pulse_capture_read(ir_pulse_capture_t *cap, ir_pulse_t *pulse);
void ir_pulse_capture_deinit(ir_pulse_capture_t *cap);

#ifdef __cplusplus
}
#endif

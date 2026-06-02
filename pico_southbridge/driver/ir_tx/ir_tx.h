#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "hardware/pio.h"

typedef struct {
  PIO pio;
  uint sm;
  uint offset;
  uint pin;
  bool initialized;
} ir_tx_t;

typedef struct {
  bool mark;
  uint32_t duration_us;
} ir_tx_pulse_t;

#ifdef __cplusplus
extern "C" {
#endif

bool ir_tx_init(ir_tx_t *tx, PIO pio, uint pin);
void ir_tx_deinit(ir_tx_t *tx);

void ir_tx_start(ir_tx_t *tx);
void ir_tx_stop(ir_tx_t *tx);

bool ir_tx_write_mark_us(ir_tx_t *tx, uint32_t duration_us);
bool ir_tx_write_space_us(ir_tx_t *tx, uint32_t duration_us);
bool ir_tx_write_pulse(ir_tx_t *tx, bool mark, uint32_t duration_us);
bool ir_tx_write_raw(ir_tx_t *tx, const ir_tx_pulse_t *pulses, uint32_t count);

bool ir_tx_fifo_full(ir_tx_t *tx);
bool ir_tx_fifo_empty(ir_tx_t *tx);

#ifdef __cplusplus
}
#endif

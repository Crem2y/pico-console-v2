#include "ir_tx.h"

#include "hardware/clocks.h"
#include "hardware/gpio.h"
#include "ir_tx.pio.h"

#define IR_TX_CARRIER_HZ        38000u
#define IR_TX_PIO_CLK_HZ        (IR_TX_CARRIER_HZ * 8u)
#define IR_TX_HALF_CYCLES_PER_S (IR_TX_CARRIER_HZ * 2u)

#define IR_TX_WORD_MARK(n)      (((n) << 1) | 1u)
#define IR_TX_WORD_SPACE(n)     (((n) << 1) | 0u)

static uint32_t ir_tx_us_to_half_cycles(uint32_t duration_us) {
  uint64_t v = (uint64_t)duration_us * IR_TX_HALF_CYCLES_PER_S;

  // round: us * 76000 / 1000000
  uint32_t cycles = (uint32_t)((v + 500000u) / 1000000u);

  if (cycles == 0) {
    cycles = 1;
  }

  return cycles;
}

static uint32_t ir_tx_make_word(bool mark, uint32_t duration_us) {
  uint32_t half_cycles = ir_tx_us_to_half_cycles(duration_us);

  // PIO의 jmp x-- 루프 특성상 count - 1을 넣는다.
  uint32_t encoded_count = half_cycles - 1u;

  if (mark) {
    return IR_TX_WORD_MARK(encoded_count);
  } else {
    return IR_TX_WORD_SPACE(encoded_count);
  }
}

bool ir_tx_init(ir_tx_t *tx, PIO pio, uint pin) {
  if (tx == NULL) return false;

  int sm = pio_claim_unused_sm(pio, false);
  if (sm < 0) return false;

  uint offset = pio_add_program(pio, &ir_tx_program);

  tx->pio = pio;
  tx->sm = (uint)sm;
  tx->offset = offset;
  tx->pin = pin;
  tx->initialized = true;

  pio_gpio_init(pio, pin);
  pio_sm_set_consecutive_pindirs(pio, tx->sm, pin, 1, true);

  pio_sm_config c = ir_tx_program_get_default_config(offset);

  sm_config_set_set_pins(&c, pin, 1);
  sm_config_set_out_shift(&c, true, false, 32);

  // TX FIFO length = 8
  sm_config_set_fifo_join(&c, PIO_FIFO_JOIN_TX);

  float div = (float)clock_get_hz(clk_sys) / (float)IR_TX_PIO_CLK_HZ;
  sm_config_set_clkdiv(&c, div);

  pio_sm_init(pio, tx->sm, offset, &c);
  pio_sm_set_enabled(pio, tx->sm, false);
  pio_sm_clear_fifos(pio, tx->sm);

  return true;
}

void ir_tx_deinit(ir_tx_t *tx) {
  if (tx == NULL || !tx->initialized) return;

  pio_sm_set_enabled(tx->pio, tx->sm, false);
  pio_remove_program(tx->pio, &ir_tx_program, tx->offset);
  pio_sm_unclaim(tx->pio, tx->sm);

  tx->initialized = false;
}

void ir_tx_start(ir_tx_t *tx) {
  if (tx == NULL || !tx->initialized) return;

  pio_sm_set_enabled(tx->pio, tx->sm, false);
  pio_sm_clear_fifos(tx->pio, tx->sm);
  pio_sm_restart(tx->pio, tx->sm);
  pio_sm_set_enabled(tx->pio, tx->sm, true);
}

void ir_tx_stop(ir_tx_t *tx) {
  if (tx == NULL || !tx->initialized) return;

  pio_sm_set_enabled(tx->pio, tx->sm, false);
}

bool ir_tx_fifo_full(ir_tx_t *tx) {
  if (tx == NULL || !tx->initialized) return true;

  return pio_sm_is_tx_fifo_full(tx->pio, tx->sm);
}

bool ir_tx_fifo_empty(ir_tx_t *tx) {
  if (tx == NULL || !tx->initialized) return true;

  return pio_sm_is_tx_fifo_empty(tx->pio, tx->sm);
}

bool ir_tx_write_pulse(ir_tx_t *tx, bool mark, uint32_t duration_us) {
  if (tx == NULL || !tx->initialized) return false;

  uint32_t word = ir_tx_make_word(mark, duration_us);

  pio_sm_put_blocking(tx->pio, tx->sm, word);
  return true;
}

bool ir_tx_write_mark_us(ir_tx_t *tx, uint32_t duration_us) {
  return ir_tx_write_pulse(tx, true, duration_us);
}

bool ir_tx_write_space_us(ir_tx_t *tx, uint32_t duration_us) {
  return ir_tx_write_pulse(tx, false, duration_us);
}

bool ir_tx_write_raw(ir_tx_t *tx, const ir_tx_pulse_t *pulses, uint32_t count) {
  if (tx == NULL || !tx->initialized || pulses == NULL) return false;

  for (uint32_t i = 0; i < count; i++) {
    ir_tx_write_pulse(tx, pulses[i].mark, pulses[i].duration_us);
  }

  return true;
}
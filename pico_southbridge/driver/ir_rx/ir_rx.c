#include "ir_rx.h"

#include "hardware/clocks.h"
#include "hardware/gpio.h"
#include "ir_rx_capture.pio.h"

#define IR_PULSE_CAPTURE_PIO_CLK_HZ 2000000u

bool ir_pulse_capture_init(ir_pulse_capture_t *cap, PIO pio, uint pin) {
  if (cap == NULL) return false;

  int sm = pio_claim_unused_sm(pio, false);
  if (sm < 0) return false;

  uint offset = pio_add_program(pio, &ir_pulse_capture_program);

  cap->pio = pio;
  cap->sm = (uint)sm;
  cap->offset = offset;
  cap->pin = pin;
  cap->current_level = false;
  cap->running = false;

  gpio_init(pin);
  gpio_set_dir(pin, GPIO_IN);
  gpio_pull_up(pin);

  pio_sm_config c = ir_pulse_capture_program_get_default_config(offset);

  sm_config_set_jmp_pin(&c, pin);

  // RX FIFO length = 8
  sm_config_set_fifo_join(&c, PIO_FIFO_JOIN_RX);

  // stable loop = 2 instruction
  // PIO clock = 2MHz → 1 loop = 1us
  float div = (float)clock_get_hz(clk_sys) / (float)IR_PULSE_CAPTURE_PIO_CLK_HZ;
  sm_config_set_clkdiv(&c, div);

  pio_sm_set_consecutive_pindirs(pio, (uint)sm, pin, 1, false);
  pio_sm_init(pio, (uint)sm, offset, &c);
  pio_sm_set_enabled(pio, (uint)sm, false);

  return true;
}

void ir_pulse_capture_start(ir_pulse_capture_t *cap) {
  if (cap == NULL) return;

  pio_sm_set_enabled(cap->pio, cap->sm, false);
  pio_sm_clear_fifos(cap->pio, cap->sm);
  pio_sm_restart(cap->pio, cap->sm);

  cap->current_level = gpio_get(cap->pin);
  cap->running = true;

  pio_sm_set_enabled(cap->pio, cap->sm, true);
}

void ir_pulse_capture_stop(ir_pulse_capture_t *cap) {
  if (cap == NULL) return;

  pio_sm_set_enabled(cap->pio, cap->sm, false);
  cap->running = false;
}

bool ir_pulse_capture_available(ir_pulse_capture_t *cap) {
  if (cap == NULL) return false;

  return !pio_sm_is_rx_fifo_empty(cap->pio, cap->sm);
}

bool ir_pulse_capture_read(ir_pulse_capture_t *cap, ir_pulse_t *pulse) {
  if (cap == NULL || pulse == NULL) return false;
  if (pio_sm_is_rx_fifo_empty(cap->pio, cap->sm)) return false;

  uint32_t raw = pio_sm_get(cap->pio, cap->sm);

  pulse->level = cap->current_level;
  pulse->duration_us = 0xffffffffu - raw;

  cap->current_level = !cap->current_level;

  return true;
}

void ir_pulse_capture_deinit(ir_pulse_capture_t *cap) {
  if (cap == NULL) return;

  pio_sm_set_enabled(cap->pio, cap->sm, false);
  pio_remove_program(cap->pio, &ir_pulse_capture_program, cap->offset);
  pio_sm_unclaim(cap->pio, cap->sm);

  cap->running = false;
}
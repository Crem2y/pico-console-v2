#include "ir_tx_pio.h"

static PIO tx_pio;
static uint tx_gpio;
static int tx_sm;

void ir_tx_pio_init(PIO pio, int tx_pin) {
    tx_pio = pio;                                 // choose which PIO block to use (RP2040 has two: pio0 and pio1)
    tx_gpio = tx_pin;                              // choose which GPIO pin is connected to the IR LED
    tx_sm = nec_tx_init(tx_pio, tx_gpio);         // uses two state machines, 16 instructions and one IRQ
}

void ir_tx_pio_send(uint8_t addr, uint8_t cmd) {
    uint32_t tx_frame = nec_encode_frame(addr, cmd);
    pio_sm_put(tx_pio, tx_sm, tx_frame);
}
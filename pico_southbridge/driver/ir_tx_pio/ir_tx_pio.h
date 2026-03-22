#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "nec_transmit.h"

#ifdef __cplusplus
extern "C" {
#endif

void ir_tx_pio_init(PIO pio, int tx_pin);
void ir_tx_pio_send(uint8_t addr, uint8_t cmd);


#ifdef __cplusplus
}
#endif
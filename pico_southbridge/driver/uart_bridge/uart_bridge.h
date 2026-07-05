#pragma once

#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/irq.h"

#define UART_BRIDGE_BUF_SIZE 128

typedef struct _bridge_queue_t {
  volatile char* buf;
  size_t buf_size;
  size_t head;
  size_t tail;
} bridge_queue_t;

#ifdef __cplusplus
extern "C" {
#endif

void uart_bridge_init(uart_inst_t* _uart, int tx_pin, int rx_pin, int baudrate);

void uart_bridge_enable_irq(void);
void uart_bridge_disable_irq(void);

int uart_bridge_readable(void);
int uart_bridge_writable(void);

int uart_bridge_write(const uint8_t* data, size_t data_size);
int uart_bridge_read(uint8_t* data, size_t buf_size);

#ifdef __cplusplus
}
#endif
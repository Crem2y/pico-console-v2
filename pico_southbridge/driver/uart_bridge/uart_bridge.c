#include <stdio.h>
#include "uart_bridge.h"

static uart_inst_t* _uart;
static int _tx_pin;
static int _rx_pin;
static int _baudrate;

volatile bridge_queue_t tx_queue;
volatile bridge_queue_t rx_queue;

volatile char bridge_tx_buf[UART_BRIDGE_BUF_SIZE];
volatile char bridge_rx_buf[UART_BRIDGE_BUF_SIZE];

void uart_irq_tx(void) {
  while (uart_is_writable(_uart) && tx_queue.head != tx_queue.tail) {
    uart_putc(_uart, tx_queue.buf[tx_queue.head]);
    tx_queue.head = (tx_queue.head + 1) % tx_queue.buf_size;
  }
}

void uart_irq_rx(void) {
  while (uart_is_readable(_uart) && ((rx_queue.tail + 1) % rx_queue.buf_size) != rx_queue.head) {
    rx_queue.buf[rx_queue.tail] = uart_getc(_uart);
    rx_queue.tail = (rx_queue.tail + 1) % rx_queue.buf_size;
  }
}

void on_uart_irq(void) {
  if (uart_is_readable(_uart)) {
    uart_irq_rx();
  }
  // if (uart_is_writable(_uart)) {
  //   uart_irq_tx();
  // }
}

void uart_bridge_init(uart_inst_t* uart, int tx_pin, int rx_pin, int baudrate) {
  _uart = uart;
  _tx_pin = tx_pin;
  _rx_pin = rx_pin;
  _baudrate = baudrate;

  tx_queue.buf = bridge_tx_buf;
  tx_queue.buf_size = sizeof(bridge_tx_buf);
  tx_queue.head = 0;
  tx_queue.tail = 0;

  rx_queue.buf = bridge_rx_buf;
  rx_queue.buf_size = sizeof(bridge_rx_buf);
  rx_queue.head = 0;
  rx_queue.tail = 0;

  irq_num_t irq_num;
  if(_uart == uart0) {
    irq_num = UART0_IRQ;
  } else if(_uart == uart1) {
    irq_num = UART1_IRQ;
  } else {
    // invalid uart instance
    return;
  }

  uart_init(_uart, _baudrate);
  gpio_set_function(_tx_pin, GPIO_FUNC_UART);
  gpio_set_function(_rx_pin, GPIO_FUNC_UART);

  irq_set_exclusive_handler(irq_num, on_uart_irq);
  //uart_bridge_enable_irq();
}

void uart_bridge_enable_irq(void) {
  irq_num_t irq_num;
  if(_uart == uart0) {
    irq_num = UART0_IRQ;
  } else if(_uart == uart1) {
    irq_num = UART1_IRQ;
  } else {
    // invalid uart instance
    return;
  }

  irq_set_enabled(irq_num, true);
  uart_set_irq_enables(_uart, true, false);
}

void uart_bridge_disable_irq(void) {
  irq_num_t irq_num;
  if(_uart == uart0) {
    irq_num = UART0_IRQ;
  } else if(_uart == uart1) {
    irq_num = UART1_IRQ;
  } else {
    // invalid uart instance
    return;
  }

  irq_set_enabled(irq_num, false);
  uart_set_irq_enables(_uart, false, false);
}

/**
 * @brief return available data size
 * 
 * @return int data size (0-UART_BRIDGE_BUF_SIZE)
 */
int uart_bridge_readable(void) {
  return (rx_queue.tail + rx_queue.buf_size - rx_queue.head) % rx_queue.buf_size;
}

/**
 * @brief return available buffer size
 * 
 * @return int buffer size (0-UART_BRIDGE_BUF_SIZE)
 */
int uart_bridge_writable(void) {
  return (tx_queue.head + tx_queue.buf_size - tx_queue.tail) % tx_queue.buf_size;
}

int uart_bridge_send(size_t data_size, const uint8_t* data) {
  if (data_size > UART_BRIDGE_BUF_SIZE || data == NULL) {
    return -1; // Data size exceeds buffer size or data is NULL
  }
  uart_write_blocking(_uart, data, data_size);
  return 1; // Success
}

int uart_bridge_receive(size_t buf_size, uint8_t* data) {
  if (buf_size > UART_BRIDGE_BUF_SIZE || data == NULL) {
    return -1; // Buffer size exceeds buffer capacity or data is NULL
  }

}
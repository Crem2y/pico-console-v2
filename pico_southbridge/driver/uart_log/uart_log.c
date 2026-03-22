#include "uart_log.h"

static uart_inst_t* _uart;
static int _tx_pin;
static int _rx_pin;
static int _baudrate;

volatile char rx_buf[64];
volatile int rx_idx = 0;

int _write(int fd, const void *buf, size_t count) {
  const char *cbuf = (const char *)buf;
  for (size_t i = 0; i < count; ++i) {
    uart_putc_raw(_uart, cbuf[i]);
  }
  return count;
}

void on_uart_rx(void) {
  while (uart_is_readable(_uart)) {
    char c = uart_getc(_uart);
    //uart_putc_raw(_uart, c); // loopback test

    rx_buf[rx_idx++] = c;
    if (rx_idx >= sizeof(rx_buf)) rx_idx = 0;
  }
}

void uartLog_init(uart_inst_t* uart, int tx_pin, int rx_pin, int baudrate) {
  _uart = uart;
  _tx_pin = tx_pin;
  _rx_pin = rx_pin;
  _baudrate = baudrate;

  uart_init(_uart, _baudrate);
  gpio_set_function(_tx_pin, GPIO_FUNC_UART);
  gpio_set_function(_rx_pin, GPIO_FUNC_UART);

  irq_set_exclusive_handler(UART0_IRQ, on_uart_rx);
  irq_set_enabled(UART0_IRQ, true);

  uart_set_irq_enables(_uart, true, false); // RX: true, TX: false

  setlocale(LC_ALL,"");
}

void uartLog_set_baudrate(int baudrate) {
  _baudrate = baudrate;
  uart_init(_uart, _baudrate);
}
#include "uart_log.h"

static uart_inst_t* _uart;
static int _tx_pin;
static int _rx_pin;
static int _baudrate;

volatile char rx_buf[64];
volatile int rx_idx = 0;

enum log_level log_lvl;

int _write(int fd, const void *buf, size_t count) {
  const char *cbuf = (const char *)buf;
  for (size_t i = 0; i < count; ++i) {
    uart_putc_raw(_uart, cbuf[i]);
  }
  return count;
}

void uartLog_irq(void) {
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

  irq_set_exclusive_handler(UART0_IRQ, uartLog_irq);
  irq_set_enabled(UART0_IRQ, true);

  uart_set_irq_enables(_uart, true, false); // RX: true, TX: false

  uartLog_set_level(LOG_DEFAULT_LEVEL);
}

void uartLog_set_baudrate(int baudrate) {
  _baudrate = baudrate;
  uart_init(_uart, _baudrate);
}

void uartLog_set_level(enum log_level level) {
  log_lvl = level;
}

void uartLog_print(const char *s) {
  while(*s) {
    uart_putc_raw(_uart, *s);
    s++;
  }
}

int uartLog_printf(enum log_level level, const char *format, ...) {
  if(level > log_lvl) return 0;

#if LOG_PRINT_WITH_LEVEL
  switch(level) {
    case LOG_ERROR:
      uartLog_print("[ERR]");
      break;
    case LOG_WARN:
      uartLog_print("[WARN]");
      break;
    case LOG_INFO:
      uartLog_print("[INFO]");
      break;
    case LOG_DEBUG:
      uartLog_print("[DBG]");
      break;
    case LOG_TRACE:
      uartLog_print("[TRACE]");
      break;
  }
#endif

  va_list args;
  va_start(args, format);

  int result = uartLog_vprintf(format, args);

  return result;
}

int uartLog_vprintf(const char *format, va_list args) {
  char string_buf[LOG_PRINTF_BUFFER_SIZE];

  int len = vsnprintf(
    string_buf,
    sizeof(string_buf),
    format,
    args
  );

  if (len >= 0) {
    uartLog_print(string_buf);
  }

  return len;
}
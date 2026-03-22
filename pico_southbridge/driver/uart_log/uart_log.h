#pragma once

#include <stdio.h>
// for using unicode
#include <wchar.h>
#include <locale.h>

#include "pico/stdlib.h"
#include "hardware/uart.h"

#define LOG_PRINTF(fmt, ...) wprintf(L##fmt, ##__VA_ARGS__)

#ifdef __cplusplus
extern "C" {
#endif

void uartLog_init(uart_inst_t* _uart, int tx_pin, int rx_pin, int baudrate);
void uartLog_set_baudrate(int baudrate);
void on_uart_rx(void);

#ifdef __cplusplus
}
#endif
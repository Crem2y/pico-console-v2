#pragma once

#include "pico/stdlib.h"
#include "hardware/uart.h"

#include <string.h>
#include "bridge_commands.h"

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

int uart_bridge_send(size_t data_size, const uint8_t* data);
int uart_bridge_receive(size_t buf_size, uint8_t* data);

#ifdef __cplusplus
}
#endif

////////////////////////////////////////////////////////////////////////////////

#define BRIDGE_CMD_QUEUE_SIZE 32
#define PAYLOAD_MAX_SIZE 16
#define BRIDGE_HEADER 0xAA
#define BRIDGE_TAIL 0x55

typedef struct _bridge_protocol_t {
  uint8_t header;                     // 0xAA
  enum bridge_cmd cmd;                // command code
  uint8_t payload_size;               // max 16
  uint8_t payload[PAYLOAD_MAX_SIZE];  // max payload size is 16 bytes
  uint8_t checksum;                   // XOR of cmd and payload
  uint8_t tail;                       // 0x55
} bridge_protocol_t;

typedef enum {
  SEQ_WAIT_HEADER = 0,
  SEQ_WAIT_COMMAND,
  SEQ_WAIT_LENGTH,
  SEQ_WAIT_PAYLOAD,
  SEQ_WAIT_INTEGRITY,
  SEQ_WAIT_TAIL
} ProtocolSequence;

#ifdef __cplusplus
extern "C" {
#endif

bridge_protocol_t bridge_protocol_create(enum bridge_cmd cmd, size_t payload_size, uint8_t* payload);
int bridge_cmd_queue_push(bridge_protocol_t cmd);
int bridge_cmd_queue_pop(bridge_protocol_t* cmd);

void bridge_protocol_parse(uint8_t* data, size_t data_size);
inline void bridge_protocol_error_print(ProtocolSequence error_seq, uint8_t data);

void set_bridge_do_cmd(void (*do_cmd)(bridge_protocol_t*));

void bridge_handle(void);

#ifdef __cplusplus
}
#endif
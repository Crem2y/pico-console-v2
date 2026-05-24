#pragma once

#include <string.h>
#include "bridge_commands.h"
#include "uart_bridge.h"

#define BRIDGE_CMD_QUEUE_SIZE 32
#define PAYLOAD_MAX_SIZE 16
#define BRIDGE_HEADER 0xAA
#define BRIDGE_TAIL 0x55

typedef struct _bridge_msg_t {
  uint8_t cmd;                        // command code
  uint8_t payload_size;               // max 16
  uint8_t payload[PAYLOAD_MAX_SIZE];  // max payload size is 16 bytes
} bridge_msg_t;

typedef struct _bridge_protocol_t {
  uint8_t header;                     // 0xAA
  uint8_t cmd;                        // command code
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

class bridgeProtocol {
  public:
    bridgeProtocol(void);

    void init(void);

    bridge_msg_t bridge_msg_create(enum bridge_cmd cmd, size_t payload_size, const uint8_t* payload);

    int bridge_msg_tx_queue_push(bridge_msg_t msg);
    int bridge_msg_tx_queue_pop(bridge_msg_t* msg);

    int bridge_msg_rx_queue_push(bridge_msg_t msg);
    int bridge_msg_rx_queue_pop(bridge_msg_t* msg);

    void bridge_protocol_parse(const uint8_t* data, size_t data_size);
    void bridge_protocol_execute_cmd(void);
    inline void bridge_protocol_error_print(ProtocolSequence error_seq, uint8_t data);

    void set_bridge_do_cmd(void (*do_cmd)(const bridge_msg_t*));

    bridge_protocol_t bridge_protocol_create(const bridge_msg_t* msg);
    void bridge_handle(void);

  private:
    void (*do_cmd_function)(const bridge_msg_t*);

    bridge_msg_t bridge_tx_queue[BRIDGE_CMD_QUEUE_SIZE];
    size_t bridge_tx_queue_head = 0;
    size_t bridge_tx_queue_tail = 0;

    bridge_msg_t bridge_rx_queue[BRIDGE_CMD_QUEUE_SIZE];
    size_t bridge_rx_queue_head = 0;
    size_t bridge_rx_queue_tail = 0;
};
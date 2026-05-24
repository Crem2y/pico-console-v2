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

////////////////////////////////////////////////////////////////////////////////

void (*do_cmd_function)(const bridge_msg_t*);

bridge_msg_t bridge_tx_queue[BRIDGE_CMD_QUEUE_SIZE];
size_t bridge_tx_queue_head = 0;
size_t bridge_tx_queue_tail = 0;

bridge_msg_t bridge_rx_queue[BRIDGE_CMD_QUEUE_SIZE];
size_t bridge_rx_queue_head = 0;
size_t bridge_rx_queue_tail = 0;

bridge_msg_t bridge_msg_create(enum bridge_cmd cmd, size_t payload_size, const uint8_t* payload) {
  bridge_msg_t msg;
  msg.cmd = (uint8_t)cmd;
  memset(msg.payload, 0, PAYLOAD_MAX_SIZE);
  if (payload_size > PAYLOAD_MAX_SIZE) {
    payload_size = PAYLOAD_MAX_SIZE;
  }
  if(payload != NULL) {
    memcpy(msg.payload, payload, payload_size);
  } else {
    payload_size = 0;
  }
  msg.payload_size = payload_size;

  return msg;
}

int bridge_msg_tx_queue_push(bridge_msg_t msg) {
  size_t next_tail = (bridge_tx_queue_tail + 1) % BRIDGE_CMD_QUEUE_SIZE;
  if (next_tail == bridge_tx_queue_head) {
    return -1; // Queue is full
  }
  bridge_tx_queue[bridge_tx_queue_tail] = msg;
  bridge_tx_queue_tail = next_tail;
  return 1; // Success
}

int bridge_msg_tx_queue_pop(bridge_msg_t* msg) {
  if (bridge_tx_queue_head == bridge_tx_queue_tail) {
    return -1; // Queue is empty
  }
  *msg = bridge_tx_queue[bridge_tx_queue_head];
  bridge_tx_queue_head = (bridge_tx_queue_head + 1) % BRIDGE_CMD_QUEUE_SIZE;
  return 1; // Success
}

int bridge_msg_rx_queue_push(bridge_msg_t msg) {
  size_t next_tail = (bridge_rx_queue_tail + 1) % BRIDGE_CMD_QUEUE_SIZE;
  if (next_tail == bridge_rx_queue_head) {
    return -1; // Queue is full
  }
  bridge_rx_queue[bridge_rx_queue_tail] = msg;
  bridge_rx_queue_tail = next_tail;
  return 1; // Success
}

int bridge_msg_rx_queue_pop(bridge_msg_t* msg) {
  if (bridge_rx_queue_head == bridge_rx_queue_tail) {
    return -1; // Queue is empty
  }
  *msg = bridge_rx_queue[bridge_rx_queue_head];
  bridge_rx_queue_head = (bridge_rx_queue_head + 1) % BRIDGE_CMD_QUEUE_SIZE;
  return 1; // Success
}

void bridge_protocol_parse(const uint8_t* data, size_t data_size) {
  static ProtocolSequence seq = SEQ_WAIT_HEADER;
  static bridge_msg_t msg;
  static bridge_protocol_t cmd;
  static size_t payload_index = 0;
  static uint8_t checksum = 0;

  for (size_t i = 0; i < data_size; i++) {
    uint8_t byte = data[i];
    switch (seq) {
      case SEQ_WAIT_HEADER:
        if (byte == BRIDGE_HEADER) {
          seq = SEQ_WAIT_COMMAND;
        } else {
          // Invalid header
          bridge_protocol_error_print(seq, byte);
        }
        break;
      case SEQ_WAIT_COMMAND:
        msg.cmd = byte;
        checksum = byte;
        seq = SEQ_WAIT_LENGTH;
        break;
      case SEQ_WAIT_LENGTH:
        msg.payload_size = byte;
        checksum ^= byte;
        if (msg.payload_size > PAYLOAD_MAX_SIZE) {
          // Invalid payload size, reset state
          bridge_protocol_error_print(seq, byte);
          seq = SEQ_WAIT_HEADER;
        } else if (msg.payload_size == 0) {
          seq = SEQ_WAIT_INTEGRITY; // No payload, skip to integrity check
        } else {
          payload_index = 0;
          seq = SEQ_WAIT_PAYLOAD;
        }
        break;
      case SEQ_WAIT_PAYLOAD:
        msg.payload[payload_index++] = byte;
        checksum ^= byte;
        if (payload_index >= msg.payload_size) {
          seq = SEQ_WAIT_INTEGRITY;
        }
        break;
      case SEQ_WAIT_INTEGRITY:
        uint8_t packet_checksum = byte;
        if (checksum == packet_checksum) {
          seq = SEQ_WAIT_TAIL;
        } else {
          // Invalid checksum, reset state
          printf("Bridge protocol error: expected 0x%02X, got 0x%02X\n", checksum, packet_checksum);
          //bridge_protocol_error_handle(seq, byte);
          seq = SEQ_WAIT_HEADER;
        }
        break;
      case SEQ_WAIT_TAIL:
        if (byte == BRIDGE_TAIL) {
          //printf("Bridge protocol OK! (cmd: 0x%02X, size: %d)\n", cmd.cmd, cmd.payload_size);
          bridge_msg_rx_queue_push(msg);
        } else {
          // Invalid tail
          bridge_protocol_error_print(seq, byte);
        }
        // Reset state for next command
        seq = SEQ_WAIT_HEADER;
        break;
    }
  }
}

void bridge_protocol_execute_cmd(void) {
  bridge_msg_t msg;
  while(bridge_msg_rx_queue_pop(&msg) > 0) {
    if(do_cmd_function) {
      do_cmd_function(&msg);
    }
  }
}

inline void bridge_protocol_error_print(ProtocolSequence error_seq, uint8_t data) {
  // print error message
  switch (error_seq)
  {
  case SEQ_WAIT_HEADER:
    printf("Bridge protocol error: expected header 0x%02X, got 0x%02X\n", BRIDGE_HEADER, data);
    break;
  case SEQ_WAIT_COMMAND:
    break;
  case SEQ_WAIT_LENGTH:
    printf("Bridge protocol error: expected length < %d, got 0x%02X\n", PAYLOAD_MAX_SIZE, data);
    break;
  case SEQ_WAIT_PAYLOAD:
    break;
  case SEQ_WAIT_INTEGRITY:
    printf("Bridge protocol error: expected integrity byte, got 0x%02X\n", data);
    break;
  case SEQ_WAIT_TAIL:
    printf("Bridge protocol error: expected tail 0x%02X, got 0x%02X\n", BRIDGE_TAIL, data);
    break;
  default:
    break;
  }
}

void set_bridge_do_cmd(void (*do_cmd)(const bridge_msg_t*)) {
  do_cmd_function = do_cmd;
}

bridge_protocol_t bridge_protocol_create(const bridge_msg_t* msg) {
  bridge_protocol_t packet;
  size_t payload_size = msg->payload_size;

  packet.header = BRIDGE_HEADER;
  packet.cmd = (uint8_t)msg->cmd;
  memset(packet.payload, 0, PAYLOAD_MAX_SIZE);
  if (payload_size > PAYLOAD_MAX_SIZE) {
    payload_size = PAYLOAD_MAX_SIZE;
  }
  if(msg->payload != NULL) {
    memcpy(packet.payload, msg->payload, payload_size);
  } else {
    payload_size = 0;
  }
  packet.payload_size = payload_size;
  packet.checksum = packet.cmd;
  packet.checksum ^= packet.payload_size;
  for (size_t i = 0; i < packet.payload_size; i++) {
    packet.checksum ^= packet.payload[i];
  }
  packet.tail = BRIDGE_TAIL;

  return packet;
}

void bridge_handle(void) {
  int rx_data_size = uart_bridge_readable();
  if(rx_data_size > 0) {
    uint8_t data[UART_BRIDGE_BUF_SIZE];
    for (size_t i = 0; i < rx_data_size; i++) {
      data[i] = rx_queue.buf[(rx_queue.head + i) % rx_queue.buf_size];
    }
    rx_queue.head = (rx_queue.head + rx_data_size) % rx_queue.buf_size;
    bridge_protocol_parse(data, rx_data_size);
  }

  bridge_msg_t msg;
  bridge_protocol_t packet;
  if(bridge_msg_tx_queue_pop(&msg) > 0) {
    // Send cmd via UART
    packet = bridge_protocol_create(&msg);
    uint8_t buffer[sizeof(bridge_protocol_t)];
    buffer[0] = packet.header;
    buffer[1] = packet.cmd;
    buffer[2] = packet.payload_size;
    memcpy(&buffer[3], packet.payload, packet.payload_size);
    buffer[3 + packet.payload_size] = packet.checksum;
    buffer[4 + packet.payload_size] = packet.tail;
    uart_bridge_send(5 + packet.payload_size, buffer);
  }
}
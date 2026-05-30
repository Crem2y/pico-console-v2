#include "ir_link.hpp"
#include "bridge_protocol.hpp"

extern bridgeProtocol Bridge;

irLink::irLink(void) {

}

void irLink::init(void) {

}

void irLink::tx_enable(bool enable, uint8_t format) {
  int payload_size = 1;
  uint8_t payload_buf[PAYLOAD_MAX_SIZE];

  payload_buf[0] = format;
  if (enable) {
    Bridge.bridge_msg_push(CMD_IR_TX_ENABLE, payload_size, payload_buf);
  } else {
    Bridge.bridge_msg_push(CMD_IR_TX_DISABLE, payload_size, payload_buf);
  }
}

void irLink::set_tx_data(uint8_t format, const uint8_t* data, uint8_t len) {
  if(len + 1 > PAYLOAD_MAX_SIZE) return;

  uint8_t payload_buf[PAYLOAD_MAX_SIZE];
  payload_buf[0] = format;
  for(int i = 0; i < len; i++) {
    payload_buf[i + 1] = data[i];
  }

  Bridge.bridge_msg_push(CMD_IR_TX_DATA, len + 1, payload_buf);
}

void irLink::rx_enable(bool enable, uint8_t format) {
  int payload_size = 1;
  uint8_t payload_buf[PAYLOAD_MAX_SIZE];

  payload_buf[0] = format;
  if (enable) {
    Bridge.bridge_msg_push(CMD_IR_RX_ENABLE, payload_size, payload_buf);
  } else {
    Bridge.bridge_msg_push(CMD_IR_RX_DISABLE, payload_size, payload_buf);
  }
}

enum ir_format irLink::get_rx_format(void) {
  return rx_format;
}

bool irLink::is_data_ready(void) {
  return rx_data_len > 0;
}

int irLink::get_raw_data_pulses(void) {
  if(rx_format != IR_FORMAT_MANUAL) return -1;

  // each pulse represented by 2 bytes in manual format
  return rx_data_len / 2;
}

void irLink::update_from_bridge(const uint8_t* data, uint8_t len) {
  if(len < 2) return;

  uint8_t format = data[0];
  uint8_t sequence = data[1];
  if(format == IR_FORMAT_MANUAL) {
    // handle manual format data
    rx_format = IR_FORMAT_MANUAL;
    uint8_t sequence_length = sequence & 0x0F; // lower 4 bits for sequence length
    uint8_t sequence_index = (sequence >> 4) & 0x0F; // upper 4 bits for sequence index
    if(sequence_index == 0) {
      // first packet of the sequence, reset rx buffer
      rx_data_len = 0;
    }
    for(int i = 0; i < len - 2; i++) {
      rx_data_buf[rx_data_len + i] = data[i + 2];
    }
    rx_data_len += len - 2;
  } else if(format == IR_FORMAT_NEC) {
    // handle NEC format data
    rx_format = IR_FORMAT_NEC;
  } else {
    // unknown format
  }
}
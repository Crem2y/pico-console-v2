#include "ir_communication.hpp"
#include "bridge_protocol.hpp"

extern bridgeProtocol Bridge;

irLink::irLink(void) {

}

void irLink::init(void) {

}

void irLink::tx_enable(uint8_t format) {
  int payload_size = 1;
  uint8_t payload_buf[PAYLOAD_MAX_SIZE];

  payload_buf[0] = format;
  Bridge.bridge_msg_push(CMD_IR_TX_ENABLE, payload_size, payload_buf);
}

void irLink::tx_disable(void) {
  Bridge.bridge_msg_push(CMD_IR_TX_DISABLE, 0, NULL);
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

void irLink::rx_enable(uint8_t format) {
  int payload_size = 1;
  uint8_t payload_buf[PAYLOAD_MAX_SIZE];

  payload_buf[0] = format;
  Bridge.bridge_msg_push(CMD_IR_RX_ENABLE, payload_size, payload_buf);
}

void irLink::rx_disable(void) {
  Bridge.bridge_msg_push(CMD_IR_RX_DISABLE, 0, NULL);
}

void irLink::update_from_bridge(const uint8_t* data, uint8_t len) {
  // placeholder
  if(len < 1) return;

  uint8_t format = data[0];
  if(format == IR_FORMAT_NEC) {
    // handle NEC format data
  } else if(format == IR_FORMAT_MANUAL) {
    // handle manual format data
  } else {
    // unknown format
  }
}
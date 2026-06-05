#pragma once

#include <stdint.h>
#include <stddef.h>
#include "ir_rx.h"
#include "ir_tx.h"
#include "system_time.h"

#define IR_LINK_MAX_DATA_SIZE 224 // (16-2)*16 = 224 - 1 byte for format - 1 byte for sequencing
#define IR_LINK_MAX_PULSES 256

enum ir_format {
  IR_FORMAT_MANUAL  = 0x00,
  IR_FORMAT_NEC     = 0x01,
  IR_FORMAT_UNKNOWN = 0xFF,
};

class irLink {
  public:
    irLink(ir_pulse_capture_t* ir_rx, ir_tx_t* ir_tx);

    void init(void);
    void update(void);

    void send_ir_blocking(const uint16_t* data, uint16_t len);

    void enable_tx(bool enable);
    void enable_rx(bool enable);

    void recv_bridge_enable_tx(const uint8_t* payload, size_t payload_size);
    void recv_bridge_enable_rx(const uint8_t* payload, size_t payload_size);
    void recv_bridge_tx_data(const uint8_t* payload, size_t payload_size);

    void send_bridge_rx_data(enum ir_format format, const uint8_t* data, size_t len);

  private:
    ir_pulse_capture_t* _ir_rx;
    ir_tx_t* _ir_tx;
    enum ir_format current_tx_format, current_rx_format;
    uint16_t tx_timings[IR_LINK_MAX_PULSES];
    uint16_t rx_timings[IR_LINK_MAX_PULSES];
    uint8_t rx_data[IR_LINK_MAX_DATA_SIZE];
    uint8_t tx_data[IR_LINK_MAX_DATA_SIZE];
    uint16_t tx_data_len;
    uint16_t tx_timing_len;
    bool is_data_ready;
};
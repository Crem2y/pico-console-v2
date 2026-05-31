#pragma once

#include <stdint.h>
#include "ir_rx.h"
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
    irLink(ir_pulse_capture_t* ir_cap);

    void init(void);
    void update(void);

    int parse_nec_data(void);

    void enable_tx(bool enable);
    void enable_rx(bool enable);

    void get_bridge_enable_tx(const uint8_t* data, uint8_t len);
    void get_bridge_enable_rx(const uint8_t* data, uint8_t len);
    void get_bridge_tx_data(const uint8_t* data, uint8_t len);

    void set_bridge_rx_data(enum ir_format format, uint8_t* data, uint8_t len);

  private:
    ir_pulse_capture_t* _ir_cap;
    enum ir_format current_tx_format, current_rx_format;
    uint16_t tx_timings[IR_LINK_MAX_PULSES];
    uint16_t rx_timings[IR_LINK_MAX_PULSES];
    uint8_t rx_data[IR_LINK_MAX_DATA_SIZE];
};
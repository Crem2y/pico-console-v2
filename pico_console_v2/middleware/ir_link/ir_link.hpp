#pragma once

#include <stdint.h>

enum ir_format {
  IR_FORMAT_MANUAL  = 0x00,
  IR_FORMAT_NEC     = 0x01,
  IR_FORMAT_UNKNOWN = 0xFF,
};

class irLink {
  public:
    irLink(void);

    void init(void);

    void tx_enable(uint8_t format);
    void tx_disable(void);
    void set_tx_data(uint8_t format, const uint8_t* data, uint8_t len);
    void rx_enable(uint8_t format);
    void rx_disable(void);

    void update_from_bridge(const uint8_t* data, uint8_t len);
};
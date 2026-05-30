#pragma once

#include <stdint.h>

#define IR_LINK_MAX_DATA_SIZE 224 // (16-2)*16 = 224 - 1 byte for format - 1 byte for sequencing

enum ir_format {
  IR_FORMAT_MANUAL  = 0x00,
  IR_FORMAT_NEC     = 0x01,
  IR_FORMAT_UNKNOWN = 0xFF,
};

class irLink {
  public:
    uint8_t tx_data_buf[IR_LINK_MAX_DATA_SIZE];
    int tx_data_len;
    uint8_t rx_data_buf[IR_LINK_MAX_DATA_SIZE];
    int rx_data_len;
    enum ir_format rx_format;

    irLink(void);

    void init(void);

    void tx_enable(bool enable, uint8_t format);
    void set_tx_data(uint8_t format, const uint8_t* data, uint8_t len);

    void rx_enable(bool enable, uint8_t format);
    enum ir_format get_rx_format(void);
    bool is_data_ready(void);
    int get_raw_data_pulses(void);

    void update_from_bridge(const uint8_t* data, uint8_t len);
  private:

};
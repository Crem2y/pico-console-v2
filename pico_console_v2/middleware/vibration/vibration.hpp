#pragma once

#include <stdint.h>
#include "system_time.h"

enum vibration_channel {
  VIBRATION_CH1 = 0,
  VIBRATION_CH2
};

class vibration {
  public:
    vibration(void);

    void init(void);
    void set_vibration(enum vibration_channel channel, uint16_t freq, uint8_t power);

    int make_bridge_payload(uint8_t* payload_buf, uint max_size);
  private:
    uint16_t freq[2];
    uint8_t power[2];
};
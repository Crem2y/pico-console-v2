#pragma once

#include <stdint.h>
#include "vibration_lra.hpp"

class vibration {
  public:
    vibration(vibrationLRA* lra);
    void init();
    void enable(bool enable);

    void recv_bridge_data(const uint8_t* payload, uint8_t payload_size);

  private:
    vibrationLRA* lra;
};
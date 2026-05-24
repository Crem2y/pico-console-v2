#pragma once

#include <stdint.h>
#include "vibration_lra.hpp"

class vibration {
  public:
    vibration(vibrationLRA* lra);
    void init();
    void enable(bool enable);

    void update_from_bridge(const uint8_t* data, uint8_t len);

  private:
    vibrationLRA* lra;
};
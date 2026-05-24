#pragma once

#include <stdint.h>
#include "i2s_pcm.h"

class audioSystem {
  public:
    audioSystem(void);

    void init(void);
    void update_from_bridge(const uint8_t* data, uint8_t len);

  private:

};
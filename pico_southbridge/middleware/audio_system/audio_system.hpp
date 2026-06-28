#pragma once

#include <stdint.h>
#include <string.h>
#include "i2s_pcm.h"

class audioSystem {
  public:
    audioSystem(void);

    void init(void);
    void recv_bridge_note_data(const uint8_t* payload, uint8_t payload_size);
    void recv_bridge_set_wave(const uint8_t* payload, uint8_t payload_size);
    void recv_bridge_set_env(const uint8_t* payload, uint8_t payload_size);
    void recv_bridge_set_master(const uint8_t* payload, uint8_t payload_size);

  private:

};
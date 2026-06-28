#pragma once

#include <stdint.h>
#include "system_time.h"

typedef struct _bridge_info_t {
  uint16_t hw_ver;
  uint16_t sw_ver;
  uint32_t hw_support;
  uint32_t sw_support;
} bridge_info_t;

class bridgeControl {
  public:
    bool connected;
    bridge_info_t my_info;

    bridgeControl(void);

    void init(void);
    void update(void);
    void update_last_comm_time(void) {
      last_bridge_comm_time = get_system_time_ms();
    }

    void send_bridge_hw_info_res(void);
    void send_bridge_sw_info_res(void);

  private:
    time_ms_t last_bridge_comm_time;
};

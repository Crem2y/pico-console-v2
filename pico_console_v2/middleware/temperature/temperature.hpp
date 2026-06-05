#pragma once

#include "temp_builtin.h"

#define TEMP_CH_NUM 2

typedef int32_t temp_t;

enum temp_ch {
  TEMP_BUILTIN = 0,
  TEMP_SOUTHBRIDGE,
};

class temperature {
  public:
    temperature(void);

    void init(void);
    void update(void);
    void recv_bridge_data(const uint8_t* payload, uint8_t payload_size);

    temp_t get_temp(temp_ch ch);

  private:
    temp_t temp[TEMP_CH_NUM];
};
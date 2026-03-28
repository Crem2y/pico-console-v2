#pragma once

#include "temp_builtin.h"

#define TEMP_CH_NUM 1

typedef int32_t temp_t;

enum temp_ch {
  TEMP_BUILTIN = 0,
  TEMP_01,
};

class temperature {
  public:
    temperature(void);

    void init(void);
    void update(void);

    int make_bridge_payload(uint8_t* payload_buf, uint max_size);

    temp_t get_temp(temp_ch ch);

  private:
    temp_t temp[TEMP_CH_NUM];
};
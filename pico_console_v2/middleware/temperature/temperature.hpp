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
    void update_from_bridge(uint8_t* data, uint8_t len);

    temp_t get_temp(temp_ch ch);

  private:
    temp_t temp[TEMP_CH_NUM];
};
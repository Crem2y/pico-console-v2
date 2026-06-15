#pragma once

#include "temp_builtin.h"

#define TEMP_CH_NUM 1

enum temp_ch {
  TEMP_BUILTIN = 0,
  TEMP_01,
};

class temperature {
  public:
    temperature(void);

    void init(void);
    void update(void);

    float get_temp(temp_ch ch);

    void send_bridge_data(void);

  private:
    float temp[TEMP_CH_NUM];
};
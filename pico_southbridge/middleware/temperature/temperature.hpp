#pragma once

#include "temp_builtin.h"
#include "temp_ntc.hpp"

#define TEMP_CH_NUM 2

enum temp_ch {
  TEMP_BUILTIN = 0,
  TEMP_NTC,
};

class temperature {
  public:
    temperature(tempNTC* ntc);

    void init(void);
    void update(void);

    float get_temp(temp_ch ch);

    void send_bridge_data(void);

  private:
    float temp[TEMP_CH_NUM];

    tempNTC* Ntc;
};
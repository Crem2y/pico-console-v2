#pragma once

#include "temp_builtin.h"
#include "temp_ntc.hpp"

enum temp_ch {
  TEMP_BUILTIN = 0,
  TEMP_NTC,
  TEMP_CH_NUM
};

class temperature {
  public:
    temperature(tempNTC* ntc);

    void init(void);
    void update(void);

    void set_temp(temp_ch ch, float temp_data);
    float get_temp(temp_ch ch);

    void send_bridge_data(void);

  private:
    float temp[TEMP_CH_NUM];

    tempNTC* Ntc;
};
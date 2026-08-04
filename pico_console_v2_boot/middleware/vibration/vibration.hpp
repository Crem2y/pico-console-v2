#pragma once

#include <stdint.h>
#include "system_time.h"

typedef struct _vibration_data_t {
  uint16_t freq;
  uint8_t power;
} vibration_data_t;

enum vibration_channel {
  VIB_L = 0,
  VIB_R,
  VIB_CH_NUM,
};

class vibration {
  public:
    vibration(void);

    void init(void);
    void update(void);
    void set_vibration(enum vibration_channel channel, uint16_t freq, uint8_t power);
    void set_freq(enum vibration_channel channel, uint16_t freq);
    void set_power(enum vibration_channel channel, uint8_t power);

    void send_bridge_vibration_data(uint16_t l_freq, uint8_t l_power, uint16_t r_freq, uint8_t r_power);
  private:
    vibration_data_t data[VIB_CH_NUM];
    vibration_data_t prev_data[VIB_CH_NUM];
};
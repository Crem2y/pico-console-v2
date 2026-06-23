#pragma once

#include "adc_vsense.hpp"

class power {
  public:
    power(AdcVSense* vin);

    void init(void);
    void update(void);

    float get_input_voltage(void) {
      return input_voltage;
    }

    void send_bridge_power_status(void);

  private:
    AdcVSense* Vin;

    float input_voltage;
};

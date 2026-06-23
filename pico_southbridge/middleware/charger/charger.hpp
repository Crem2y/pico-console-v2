#pragma once

#include "adc_vsense.hpp"
#include "bq25619.hpp"

class charger {
  public:
    charger(AdcVSense* vbat, bq25619* bq25619);

    void init(void);
    void update(void);

    float get_bat_voltage(void) {
      return bat_voltage;
    }
    float get_bat_level(void) {
      return bat_level;
    }

    bool get_charging_status(void) {
      return charging;
    }
    uint8_t get_fault_status(void)  {
      return fault;
    }

    void send_bridge_bat_status(void);

    void recv_bridge_bat_control(const uint8_t* payload, uint8_t payload_size);

  private:
    AdcVSense* Vbat;
    bq25619* Bq25619;

    float bat_voltage;
    float bat_level;

    bool is_battery_exist;

    bool charging;
    uint8_t fault;
};

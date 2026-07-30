#pragma once

#include "uart_log.h"
#include "adc_vsense.hpp"
#include "bq25619.hpp"

class charger {
  public:
    charger(AdcVSense* vbat, bq25619* bq25619);

    void init(void);
    void update(void);

    void enable_charge(bool enable) {
      Bq25619->enable_charge(enable);
    }
    
    void set_max_charge_current(uint16_t charge_ma) {
      Bq25619->set_charge_current(charge_ma);
    }
    uint16_t get_max_charge_current(void) {
      return Bq25619->get_charge_current();
    }

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

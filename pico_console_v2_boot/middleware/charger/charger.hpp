#pragma once

#include <stdint.h>

class charger {
  public:
    charger(void);

    void init(void);
    void update(void);

    void enable_charge(bool enable);
    
    void set_max_charge_current(uint16_t charge_ma);

    uint16_t get_max_charge_current(void) {
      return max_charge_current;
    }

    bool get_charge_enable(void) {
      return charge_enable;
    }
    
    float get_bat_voltage(void) {
      return voltage;
    }
    float get_bat_level(void) {
      return level;
    }

    bool get_battery_present(void) {
      return is_battery_present;
    }
    bool get_charging_status(void) {
      return is_charging;
    }
    bool get_external_power_status(void) {
      return is_external_power_present;
    }
    uint8_t get_fault_status(void)  {
      return fault;
    }

    void send_bridge_bat_control(bool enable, uint16_t charge_ma);

    void recv_bridge_bat_status(const uint8_t* payload, uint8_t payload_size);

  private:
    float voltage;
    float level;

    bool is_battery_present;
    bool is_charging;
    bool is_external_power_present;
  
    uint8_t fault;

    bool charge_enable;
    uint16_t max_charge_current;
};

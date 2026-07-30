#pragma once

#include <stdint.h>

class charger {
  public:
    charger(void);

    void init(void);
    void update(void);

    void enable_charge(bool enable) {
      charge_enable = enable;
      send_bridge_bat_control(charge_enable, max_charge_current);
    }
    
    void set_max_charge_current(uint16_t charge_ma);

    uint16_t get_max_charge_current(void) {
      return max_charge_current;
    }

    bool get_charge_enable(void) {
      return charge_enable;
    }
    
    float get_bat_voltage(void);
    float get_bat_level(void);

    bool get_battery_exist(void);
    bool get_charging_status(void);
    uint8_t get_fault_status(void);

    void send_bridge_bat_control(bool enable, uint16_t charge_ma);

    void recv_bridge_bat_status(const uint8_t* payload, uint8_t payload_size);

  private:
    float voltage;
    float level;

    bool is_battery_exist;
    bool charging;
    uint8_t fault;

    bool charge_enable;
    uint16_t max_charge_current;
};

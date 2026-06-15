#pragma once

#include "li_battery.hpp"
#include "bq25619.hpp"

class charger {
  public:
    charger(liBattery* bat, bq25619* bq25619);

    void init(void);
    void update(void);

    float get_bat_voltage(void);
    float get_bat_level(void);

    bool get_charging_status(void);
    uint8_t get_fault_status(void);

    void send_bridge_bat_status(void);

    void recv_bridge_bat_control(const uint8_t* payload, uint8_t payload_size);

  private:
    bq25619* Bq25619;
    liBattery* Bat;

    bool is_battery_exist;

    bool charging;
    uint8_t fault;
};

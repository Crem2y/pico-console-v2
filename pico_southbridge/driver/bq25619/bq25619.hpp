#pragma once

#include <stdint.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "bq25619_reg.h"

#define BQ25619_I2C_SPEED (400*1000)
#define BQ25619_DEFAULT_ADDRESS 0x6A

union bq25619_reg {
  uint8_t raw[13];
  struct {
    union bq25619_input_current_limit_reg     in_cur_lim;
    union bq25619_charger_ctrl_0_reg          ctrl_0;
    union bq25619_charge_current_limit_reg    chg_cur_lim;
    union bq25619_precharge_current_limit_reg pre_cur_lim;
    union bq25619_battery_voltage_limit_reg   bat_volt_lim;
    union bq25619_charger_ctrl_1_reg          ctrl_1;
    union bq25619_charger_ctrl_2_reg          ctrl_2;
    union bq25619_charger_ctrl_3_reg          ctrl_3;
    union bq25619_charger_status_0_reg        status_0;
    union bq25619_charger_status_1_reg        status_1;
    union bq25619_charger_status_2_reg        status_2;
    union bq25619_part_info_reg               part_info;
    union bq25619_charger_ctrl_4_reg          ctrl_4;
  };
};

class bq25619 {
  public:
    union bq25619_reg reg;

    bq25619(i2c_inst_t* i2c, int pin_sda, int pin_scl, int pin_int, uint8_t address = BQ25619_DEFAULT_ADDRESS);

    void init(void);

    void read_reg(uint8_t reg_addr);
    void read_all_regs(void);
    void reset_all_regs(void);

    void update_watchdog(void);

    void enable_charge(bool enable);
    void set_ignore_ts(bool ignore);
    void set_charge_current(uint16_t charge_current_ma);

    bool get_charging_status(void);
    bool get_external_power_status(void);
    uint8_t get_fault_status(void);
    uint16_t get_charge_current(void);

  private:
    i2c_inst_t* _i2c;
    int _pin_sda, _pin_scl, _pin_int;
    uint8_t _address;
};
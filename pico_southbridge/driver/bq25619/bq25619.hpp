#pragma once

#include <stdint.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "bq25619_reg.h"

#define BQ25619_I2C_SPEED (400*1000)
#define BQ25619_DEFAULT_ADDRESS 0x6A

class bq25619 {
  public:
    uint8_t reg_raw[13];
    union bq25619_input_current_limit_reg     reg_in_cur_lim;
    union bq25619_charger_ctrl_0_reg          reg_ctrl_0;
    union bq25619_charge_current_limit_reg    reg_chg_cur_lim;
    union bq25619_precharge_current_limit_reg reg_pre_cur_lim;
    union bq25619_battery_voltage_limit_reg   reg_bat_volt_lim;
    union bq25619_charger_ctrl_1_reg          reg_ctrl_1;
    union bq25619_charger_ctrl_2_reg          reg_ctrl_2;
    union bq25619_charger_ctrl_3_reg          reg_ctrl_3;
    union bq25619_charger_status_0_reg        reg_status_0;
    union bq25619_charger_status_1_reg        reg_status_1;
    union bq25619_charger_status_2_reg        reg_status_2;
    union bq25619_part_info_reg               reg_part_info;
    union bq25619_charger_ctrl_3_reg          reg_ctrl_3;

    bool charging;
    uint8_t fault;

    bq25619(i2c_inst_t* i2c, int pin_sda, int pin_scl, int pin_int, uint8_t address = BQ25619_DEFAULT_ADDRESS);
    void init(void);
    void update(void);

    void reset_reg(void);
    void read_all_regs(void); //test
    void set_ignore_ts(bool ignore);

  private:
    i2c_inst_t* _i2c;
    int _pin_sda, _pin_scl, _pin_int;
    uint8_t _address;
};
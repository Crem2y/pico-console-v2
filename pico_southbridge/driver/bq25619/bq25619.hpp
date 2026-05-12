#pragma once

#include <stdint.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"

#define BQ25619_I2C_SPEED (400*1000)
#define BQ25619_DEFAULT_ADDRESS 0x6A

#define BQ25619_REG_INPUT_CURRENT_LIMIT     0x00
#define BQ25619_REG_CHARGER_CONTROL_0       0x01
#define BQ25619_REG_CHARGE_CURRENT_LIMIT    0x02
#define BQ25619_REG_PRECHARGE_CURRENT_LIMIT 0x03
#define BQ25619_REG_BATTERY_VOLTAGE_LIMIT   0x04
#define BQ25619_REG_CHARGER_CONTROL_1       0x05
#define BQ25619_REG_CHARGER_CONTROL_2       0x06
#define BQ25619_REG_CHARGER_CONTROL_3       0x07
#define BQ25619_REG_CHARGER_STATUS_0        0x08
#define BQ25619_REG_CHARGER_STATUS_1        0x09
#define BQ25619_REG_CHARGER_STATUS_2        0x0A
#define BQ25619_REG_PART_INFORMATION        0x0B
#define BQ25619_REG_CHARGER_CONTROL_4       0x0C

class bq25619 {
  public:
    uint8_t reg_raw[16]; //test
    bool charging;
    uint8_t fault;
    uint8_t chrg_stat;

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
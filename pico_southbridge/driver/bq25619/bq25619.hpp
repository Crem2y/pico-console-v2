#pragma once

#include <stdint.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "bq25619_reg.h"

#define BQ25619_I2C_SPEED (400*1000)
#define BQ25619_DEFAULT_ADDRESS 0x6A

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
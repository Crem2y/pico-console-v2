#include "bq25619.hpp"

bq25619::bq25619(i2c_inst_t* i2c, int pin_sda, int pin_scl, int pin_int, uint8_t address) {
  this->_i2c = i2c;
  this->_pin_sda = pin_sda;
  this->_pin_scl = pin_scl;
  this->_pin_int = pin_int;
  this->_address = address;
}

void bq25619::init() {
  i2c_init(_i2c, BQ25619_I2C_SPEED);
  gpio_set_function(_pin_sda, GPIO_FUNC_I2C);
  gpio_set_function(_pin_scl, GPIO_FUNC_I2C);
  gpio_pull_up(_pin_sda);
  gpio_pull_up(_pin_scl);

  charging = false;
  fault = 0;
  reset_reg();
}

void bq25619::update() {
  uint8_t addr;
  uint8_t buf[2];

  // read charge status
  addr = BQ25619_REG_CHARGER_STATUS_0;
  i2c_write_blocking(_i2c, _address, &addr, 1, true);
  i2c_read_blocking(_i2c, _address, buf, 1, false);

  chrg_stat = (buf[0] & 0x18) >> 3; // bit3-4 for charge status
  if(chrg_stat == 0x01 || chrg_stat == 0x02) {
    charging = true;
  } else {
    charging = false;
  }

  // read fault status
  addr = BQ25619_REG_CHARGER_STATUS_1;
  i2c_write_blocking(_i2c, _address, &addr, 1, true);
  i2c_read_blocking(_i2c, _address, buf, 1, false);

  fault = buf[0];
}

void bq25619::reset_reg() {
  uint8_t addr = BQ25619_REG_PART_INFORMATION;
  uint8_t buf[2] = {BQ25619_REG_PART_INFORMATION, 0x80}; // register reset

  i2c_write_blocking(_i2c, _address, buf, 2, false);

  while (true) {
    i2c_write_blocking(_i2c, _address, &addr, 1, true);
    i2c_read_blocking(_i2c, _address, buf, 1, false);
    if((buf[0] & 0x80) == 0) break; // wait until reset is done
    sleep_ms(10);
  }
}

void bq25619::read_all_regs(void) {
  uint8_t addr = BQ25619_REG_INPUT_CURRENT_LIMIT;

  i2c_write_blocking(_i2c, _address, &addr, 1, true);
  i2c_read_blocking(_i2c, _address, reg_raw, 13, false);
}

void bq25619::set_ignore_ts(bool ignore) {
  uint8_t addr = BQ25619_REG_INPUT_CURRENT_LIMIT;
  uint8_t buf[2] = {BQ25619_REG_INPUT_CURRENT_LIMIT, 0x00};

  i2c_write_blocking(_i2c, _address, &addr, 1, true);
  i2c_read_blocking(_i2c, _address, &buf[1], 1, false);

  if(ignore) {
    buf[1] |= 0x40; // set bit6 to ignore TS pin
  } else {
    buf[1] &= ~0x40; // clear bit6 to enable TS pin
  }

  i2c_write_blocking(_i2c, _address, buf, 2, false);
}
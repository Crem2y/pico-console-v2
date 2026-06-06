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

  reset_all_regs();
}

void bq25619::update_watchdog() {

}

void bq25619::read_reg(uint8_t reg_addr) {
  if(reg_addr > BQ25619_REG_CHARGER_CONTROL_4) return; // invalid register address

  i2c_write_blocking(_i2c, _address, &reg_addr, 1, true);
  i2c_read_blocking(_i2c, _address, &reg.raw[reg_addr], 1, false);
}

void bq25619::read_all_regs(void) {
  uint8_t addr = BQ25619_REG_INPUT_CURRENT_LIMIT;

  i2c_write_blocking(_i2c, _address, &addr, 1, true);
  i2c_read_blocking(_i2c, _address, reg.raw, 13, false);

  reg.in_cur_lim.byte   = reg.raw[0];
  reg.ctrl_0.byte       = reg.raw[1];
  reg.chg_cur_lim.byte  = reg.raw[2];
  reg.pre_cur_lim.byte  = reg.raw[3];
  reg.bat_volt_lim.byte = reg.raw[4];
  reg.ctrl_1.byte       = reg.raw[5];
  reg.ctrl_2.byte       = reg.raw[6];
  reg.ctrl_3.byte       = reg.raw[7];
  reg.status_0.byte     = reg.raw[8];
  reg.status_1.byte     = reg.raw[9];
  reg.status_2.byte     = reg.raw[10];
  reg.part_info.byte    = reg.raw[11];
  reg.ctrl_4.byte       = reg.raw[12];
}

void bq25619::reset_all_regs() {
  uint8_t addr = BQ25619_REG_PART_INFORMATION;
  uint8_t buf[2] = {BQ25619_REG_PART_INFORMATION, 0x80}; // register reset

  i2c_write_blocking(_i2c, _address, buf, 2, false);

  while (true) {
    i2c_write_blocking(_i2c, _address, &addr, 1, true);
    i2c_read_blocking(_i2c, _address, &reg.part_info.byte, 1, false);
    if(reg.part_info.REG_RST == 0) break; // wait until reset is done
    sleep_ms(10);
  }
}

void bq25619::set_ignore_ts(bool ignore) {
  uint8_t addr = BQ25619_REG_INPUT_CURRENT_LIMIT;
  uint8_t buf[2] = {BQ25619_REG_INPUT_CURRENT_LIMIT, 0x00};

  i2c_write_blocking(_i2c, _address, &addr, 1, true);
  i2c_read_blocking(_i2c, _address, &reg.in_cur_lim.byte, 1, false);

  reg.in_cur_lim.TS_IGNORE = ignore ? 1 : 0;

  buf[1] = reg.in_cur_lim.byte;
  i2c_write_blocking(_i2c, _address, buf, 2, false);
}

bool bq25619::get_charging_status(void) {
  return reg.status_0.CHRG_STAT == 0x01 || reg.status_0.CHRG_STAT == 0x02;
}

uint8_t bq25619::get_fault_status(void) {
  return reg.status_1.byte;
}
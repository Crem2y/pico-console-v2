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
  i2c_read_blocking(_i2c, _address, &reg_status_0.byte, 1, false);

  if(reg_status_0.CHRG_STAT == 0x01 || reg_status_0.CHRG_STAT == 0x02) {
    charging = true;
  } else {
    charging = false;
  }

  // read fault status
  addr = BQ25619_REG_CHARGER_STATUS_1;
  i2c_write_blocking(_i2c, _address, &addr, 1, true);
  i2c_read_blocking(_i2c, _address, &reg_status_1.byte, 1, false);
}

void bq25619::reset_reg() {
  uint8_t addr = BQ25619_REG_PART_INFORMATION;
  uint8_t buf[2] = {BQ25619_REG_PART_INFORMATION, 0x80}; // register reset

  i2c_write_blocking(_i2c, _address, buf, 2, false);

  while (true) {
    i2c_write_blocking(_i2c, _address, &addr, 1, true);
    i2c_read_blocking(_i2c, _address, &reg_part_info.byte, 1, false);
    if(reg_part_info.REG_RST == 0) break; // wait until reset is done
    sleep_ms(10);
  }
}

void bq25619::read_all_regs(void) {
  uint8_t addr = BQ25619_REG_INPUT_CURRENT_LIMIT;

  i2c_write_blocking(_i2c, _address, &addr, 1, true);
  i2c_read_blocking(_i2c, _address, reg_raw, 13, false);

  reg_in_cur_lim.byte   = reg_raw[0];
  reg_ctrl_0.byte       = reg_raw[1];
  reg_chg_cur_lim.byte  = reg_raw[2];
  reg_pre_cur_lim.byte  = reg_raw[3];
  reg_bat_volt_lim.byte = reg_raw[4];
  reg_ctrl_1.byte       = reg_raw[5];
  reg_ctrl_2.byte       = reg_raw[6];
  reg_ctrl_3.byte       = reg_raw[7];
  reg_status_0.byte     = reg_raw[8];
  reg_status_1.byte     = reg_raw[9];
  reg_status_2.byte     = reg_raw[10];
  reg_part_info.byte    = reg_raw[11];
  reg_ctrl_3.byte       = reg_raw[12];
}

void bq25619::set_ignore_ts(bool ignore) {
  uint8_t addr = BQ25619_REG_INPUT_CURRENT_LIMIT;
  uint8_t buf[2] = {BQ25619_REG_INPUT_CURRENT_LIMIT, 0x00};

  i2c_write_blocking(_i2c, _address, &addr, 1, true);
  i2c_read_blocking(_i2c, _address, &reg_in_cur_lim.byte, 1, false);

  reg_in_cur_lim.TS_IGNORE = ignore ? 1 : 0;

  buf[1] = reg_in_cur_lim.byte;
  i2c_write_blocking(_i2c, _address, buf, 2, false);
}
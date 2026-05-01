#include "mpu6050.hpp"

mpu6050::mpu6050(i2c_inst_t* i2c, int pin_sda, int pin_scl, int pin_int, uint8_t address) {
  this->_i2c = i2c;
  this->_pin_sda = pin_sda;
  this->_pin_scl = pin_scl;
  this->_pin_int = pin_int;
  this->_address = address;
}

void mpu6050::init() {
  i2c_init(_i2c, MPU6050_I2C_SPEED);
  gpio_set_function(_pin_sda, GPIO_FUNC_I2C);
  gpio_set_function(_pin_scl, GPIO_FUNC_I2C);
  gpio_pull_up(_pin_sda);
  gpio_pull_up(_pin_scl);

  uint8_t wake_cmd[2] = {MPU6050_REG_PWR_MGMT_1, 0x80}; // software reset
  i2c_write_blocking(_i2c, _address, wake_cmd, 2, false) == 2;
  sleep_ms(100);

  wake_cmd[1] = 0x00; // wake up
  i2c_write_blocking(_i2c, _address, wake_cmd, 2, false) == 2;
  sleep_ms(100);
}

void mpu6050::read_raw_accel_data(void) {
  uint8_t addr = MPU6050_REG_ACCEL_XOUT_H;
  uint8_t raw_buf[6];

  i2c_write_blocking(_i2c, _address, &addr, 1, false);
  i2c_read_blocking(_i2c, _address, raw_buf, 6, false);

  accel_raw.x = (raw_buf[0] << 8) | raw_buf[1];
  accel_raw.y = (raw_buf[2] << 8) | raw_buf[3];
  accel_raw.z = (raw_buf[4] << 8) | raw_buf[5];
}

void mpu6050::read_raw_gyro_data(void) {
  uint8_t addr = MPU6050_REG_GYRO_XOUT_H;
  uint8_t raw_buf[6];

  i2c_write_blocking(_i2c, _address, &addr, 1, false);
  i2c_read_blocking(_i2c, _address, raw_buf, 6, false);

  gyro_raw.x = (raw_buf[0] << 8) | raw_buf[1];
  gyro_raw.y = (raw_buf[2] << 8) | raw_buf[3];
  gyro_raw.z = (raw_buf[4] << 8) | raw_buf[5];
}

void mpu6050::read_raw_temp_data(void) {
  uint8_t addr = MPU6050_REG_TEMP_OUT_H;
  uint8_t raw_buf[2];

  i2c_write_blocking(_i2c, _address, &addr, 1, false);
  i2c_read_blocking(_i2c, _address, raw_buf, 2, false);

  temp_raw = (raw_buf[0] << 8) | raw_buf[1];
} 
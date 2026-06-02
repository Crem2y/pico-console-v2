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
  i2c_write_blocking(_i2c, _address, wake_cmd, 2, false);
  sleep_ms(100);

  wake_cmd[1] = 0x00; // wake up
  i2c_write_blocking(_i2c, _address, wake_cmd, 2, false);
  sleep_ms(100);

  set_accel_range(2); // +- 2g
  set_gyro_range(250); // +- 250 degree/s

  temperature = 0.0;
}

void mpu6050::read_accel_data(void) {
  uint8_t addr = MPU6050_REG_ACCEL_XOUT_H;
  uint8_t raw_buf[6];

  i2c_write_blocking(_i2c, _address, &addr, 1, false);
  i2c_read_blocking(_i2c, _address, raw_buf, 6, false);

  accel_raw.x = (raw_buf[0] << 8) | raw_buf[1];
  accel_raw.y = (raw_buf[2] << 8) | raw_buf[3];
  accel_raw.z = (raw_buf[4] << 8) | raw_buf[5];

  float accel_scale = 32768.0f / accel_range;

  accel.x = (float)accel_raw.x / accel_scale;
  accel.y = (float)accel_raw.y / accel_scale;
  accel.z = (float)accel_raw.z / accel_scale;
}

void mpu6050::read_gyro_data(void) {
  uint8_t addr = MPU6050_REG_GYRO_XOUT_H;
  uint8_t raw_buf[6];

  i2c_write_blocking(_i2c, _address, &addr, 1, false);
  i2c_read_blocking(_i2c, _address, raw_buf, 6, false);

  gyro_raw.x = (raw_buf[0] << 8) | raw_buf[1];
  gyro_raw.y = (raw_buf[2] << 8) | raw_buf[3];
  gyro_raw.z = (raw_buf[4] << 8) | raw_buf[5];

  float gyro_scale = 32768.0f / gyro_range;

  gyro.x = (float)gyro_raw.x / gyro_scale;
  gyro.y = (float)gyro_raw.y / gyro_scale;
  gyro.z = (float)gyro_raw.z / gyro_scale;
}

void mpu6050::read_temp_data(void) {
  uint8_t addr = MPU6050_REG_TEMP_OUT_H;
  uint8_t raw_buf[2];

  i2c_write_blocking(_i2c, _address, &addr, 1, false);
  i2c_read_blocking(_i2c, _address, raw_buf, 2, false);

  temp_raw = (raw_buf[0] << 8) | raw_buf[1];
  temperature = ((float)temp_raw / 340) + 36.53f;
}

void mpu6050::set_accel_range(uint16_t range) {
  if(range != 2 && range != 4 && range != 8 && range != 16) return;

  uint8_t afs_sel;
  uint8_t cmd[2];

  if(range == 2) {
    afs_sel = 0x00;
  } else if(range == 4) {
    afs_sel = 0x01;
  } else if(range == 8) {
    afs_sel = 0x02;
  } else {
    afs_sel = 0x03;
  }

  cmd[0] = MPU6050_REG_ACCEL_CONFIG;
  cmd[1] = (afs_sel & 0x03) << 3;

  i2c_write_blocking(_i2c, _address, cmd, 2, false);
  accel_range = range;
}

void mpu6050::set_gyro_range(uint16_t range) {
  if(range != 250 && range != 500 && range != 1000 && range != 2000) return;

  uint8_t fs_sel;
  uint8_t cmd[2];

  if(range == 250) {
    fs_sel = 0x00;
  } else if(range == 500) {
    fs_sel = 0x01;
  } else if(range == 1000) {
    fs_sel = 0x02;
  } else {
    fs_sel = 0x03;
  }

  cmd[0] = MPU6050_REG_GYRO_CONFIG;
  cmd[1] = (fs_sel & 0x03) << 3;

  i2c_write_blocking(_i2c, _address, cmd, 2, false);
  gyro_range = range;
}
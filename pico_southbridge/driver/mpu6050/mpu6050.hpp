#pragma once

#include <stdint.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"

#define MPU6050_I2C_SPEED (400*1000)
#define MPU6050_DEFAULT_ADDRESS 0x68

#define MPU6050_REG_ACCEL_XOUT_H  0x3B
#define MPU6050_REG_ACCEL_XOUT_L  0x3C
#define MPU6050_REG_ACCEL_YOUT_H  0x3D
#define MPU6050_REG_ACCEL_YOUT_L  0x3E
#define MPU6050_REG_ACCEL_ZOUT_H  0x3F
#define MPU6050_REG_ACCEL_ZOUT_L  0x40

#define MPU6050_REG_TEMP_OUT_H    0x41
#define MPU6050_REG_TEMP_OUT_L    0x42

#define MPU6050_REG_GYRO_XOUT_H   0x43
#define MPU6050_REG_GYRO_XOUT_L   0x44
#define MPU6050_REG_GYRO_YOUT_H   0x45
#define MPU6050_REG_GYRO_YOUT_L   0x46
#define MPU6050_REG_GYRO_ZOUT_H   0x47
#define MPU6050_REG_GYRO_ZOUT_L   0x48

#define MPU6050_REG_PWR_MGMT_1    0x6B
#define MPU6050_REG_PWR_MGMT_2    0x6C

typedef struct _axles_int16 {
  int16_t x;
  int16_t y;
  int16_t z;
} axles_int16;

class mpu6050 {
  public:
    mpu6050(i2c_inst_t* i2c, int pin_sda, int pin_scl, int pin_int, uint8_t address = MPU6050_DEFAULT_ADDRESS);
    void init();
    void read_raw_accel_data(void);
    void read_raw_gyro_data(void);
    void read_raw_temp_data(void);

    axles_int16 accel_raw;
    axles_int16 gyro_raw;
    uint16_t temp_raw;
  private:
    i2c_inst_t* _i2c;
    int _pin_sda, _pin_scl, _pin_int;
    uint8_t _address;
};
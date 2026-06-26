#pragma once

#include <stdint.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "mpu6050_reg.h"

#define MPU6050_I2C_SPEED (400*1000)
#define MPU6050_DEFAULT_ADDRESS 0x68

typedef struct _axles_int16 {
  int16_t x;
  int16_t y;
  int16_t z;
} axles_int16;

typedef struct _axles_float {
  float x;
  float y;
  float z;
} axles_float;

class mpu6050 {
  public:
    uint16_t accel_range;
    axles_int16 accel_raw;
    axles_float accel;

    uint16_t gyro_range;
    axles_int16 gyro_raw;
    axles_float gyro;

    int16_t temp_raw;
    float temperature;

    mpu6050(i2c_inst_t* i2c, int pin_sda, int pin_scl, int pin_int, uint8_t address = MPU6050_DEFAULT_ADDRESS);
    void init();

    void read_accel_data(void);
    void read_gyro_data(void);
    void read_temp_data(void);

    void set_accel_range(uint16_t range);
    void set_gyro_range(uint16_t range);

    float get_temp_data(void) {
      return temperature;
    }

  private:
    i2c_inst_t* _i2c;
    int _pin_sda, _pin_scl, _pin_int;
    uint8_t _address;
};
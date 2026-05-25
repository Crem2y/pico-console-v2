#pragma once

#include <stdint.h>

class imu {
  public:
    uint16_t accel_raw[3];
    uint16_t gyro_raw[3];

    imu(void);

    void init(void);
    void update(void);
    void update_accel(const uint8_t* data, uint8_t len);
    void update_gyro(const uint8_t* data, uint8_t len);

    float get_accel_x();
    float get_accel_y();
    float get_accel_z();

    float get_gyro_x();
    float get_gyro_y();
    float get_gyro_z();

  private:
    float accel_x, accel_y, accel_z;
    float gyro_x, gyro_y, gyro_z;
};
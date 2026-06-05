#pragma once

#include <stdint.h>

class imu {
  public:
    int16_t accel_raw[3];
    int16_t gyro_raw[3];

    imu(void);

    void init(void);
    void update(void);
    void recv_bridge_accel_data(const uint8_t* payload, uint8_t payload_size);
    void recv_bridge_gyro_data(const uint8_t* payload, uint8_t payload_size);

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
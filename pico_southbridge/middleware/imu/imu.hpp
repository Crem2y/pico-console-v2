#pragma once

#include "mpu6050.hpp"

class imu {
  public:
    imu(mpu6050* mpu);
    void init(void);
    void update(void);
    int make_bridge_payload(uint8_t* payload_buf, uint8_t buf_size);

  private:
    mpu6050* Mpu6050;
    uint16_t accel_data_raw[3];
    uint16_t gyro_data_raw[3];
    float accel_data[3];
    float gyro_data[3];
};

#pragma once

#include <stdint.h>
#include <string.h>
#include "mpu6050.hpp"

class imu {
  public:
    imu(mpu6050* mpu);

    void init(void);
    void update(void);

    void enable_accel_data(bool enable);
    void enable_gyro_data(bool enable);

    void send_bridge_accel_data(void);
    void send_bridge_gyro_data(void);

    void recv_bridge_enable(const uint8_t* payload, uint8_t payload_size);

  private:
    mpu6050* Mpu6050;
    uint16_t accel_data_raw[3];
    uint16_t gyro_data_raw[3];
    float accel_data[3];
    float gyro_data[3];
    bool enable_accel, enable_gyro;
};

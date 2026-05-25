#include "imu.hpp"
#include "bridge_protocol.hpp"

extern bridgeProtocol Bridge;

imu::imu(mpu6050* mpu) {
  Mpu6050 = mpu;
}

void imu::init(void) {
  Mpu6050->init();
}

void imu::update(void) {
  Mpu6050->read_raw_accel_data();
  Mpu6050->read_raw_gyro_data();

  //placeholder
  accel_data_raw[0] = Mpu6050->accel_raw.x;
  accel_data_raw[1] = Mpu6050->accel_raw.y;
  accel_data_raw[2] = Mpu6050->accel_raw.z;

  gyro_data_raw[0] = Mpu6050->gyro_raw.x;
  gyro_data_raw[1] = Mpu6050->gyro_raw.y;
  gyro_data_raw[2] = Mpu6050->gyro_raw.z;

  int payload_size = 6;
  uint8_t payload_buf[PAYLOAD_MAX_SIZE];

  // accel_data
  payload_buf[0] = accel_data_raw[0] >> 8;
  payload_buf[1] = accel_data_raw[0] & 0xFF;
  payload_buf[2] = accel_data_raw[1] >> 8;
  payload_buf[3] = accel_data_raw[1] & 0xFF;
  payload_buf[4] = accel_data_raw[2] >> 8;
  payload_buf[5] = accel_data_raw[2] & 0xFF;

  Bridge.bridge_msg_push(CMD_IMU_ACCEL_DATA, payload_size, payload_buf);

  // gyro_data
  payload_buf[0] = gyro_data_raw[0] >> 8;
  payload_buf[1] = gyro_data_raw[0] & 0xFF;
  payload_buf[2] = gyro_data_raw[1] >> 8;
  payload_buf[3] = gyro_data_raw[1] & 0xFF;
  payload_buf[4] = gyro_data_raw[2] >> 8;
  payload_buf[5] = gyro_data_raw[2] & 0xFF;

  Bridge.bridge_msg_push(CMD_IMU_GYRO_DATA, payload_size, payload_buf);
}
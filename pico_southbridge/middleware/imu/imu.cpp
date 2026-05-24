#include "imu.hpp"

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
}

int imu::make_bridge_payload(uint8_t* payload_buf, uint8_t buf_size) {
  if(buf_size < 12) return -1; // need at least 12 bytes for 6 float values

  // placeholder
  // accel_data
  payload_buf[0] = accel_data_raw[0] >> 8;
  payload_buf[1] = accel_data_raw[0] & 0xFF;
  payload_buf[2] = accel_data_raw[1] >> 8;
  payload_buf[3] = accel_data_raw[1] & 0xFF;
  payload_buf[4] = accel_data_raw[2] >> 8;
  payload_buf[5] = accel_data_raw[2] & 0xFF;

  // gyro_data
  payload_buf[6] = gyro_data_raw[0] >> 8;
  payload_buf[7] = gyro_data_raw[0] & 0xFF;
  payload_buf[8] = gyro_data_raw[1] >> 8;
  payload_buf[9] = gyro_data_raw[1] & 0xFF;
  payload_buf[10] = gyro_data_raw[2] >> 8;
  payload_buf[11] = gyro_data_raw[2] & 0xFF;

  return 12;
}
#include "imu.hpp"

imu::imu(void) {

}

void imu::init(void) {

}

void imu::update(void) {

}

void imu::recv_bridge_accel_data(const uint8_t* payload, uint8_t payload_size) {
  if(payload_size < 6) return;

  accel_raw[0] = (payload[1] << 8) | payload[0];
  accel_raw[1] = (payload[3] << 8) | payload[2];
  accel_raw[2] = (payload[5] << 8) | payload[4];

  accel_x = accel_raw[0];
  accel_y = accel_raw[1];
  accel_z = accel_raw[2];
}

void imu::recv_bridge_gyro_data(const uint8_t* payload, uint8_t payload_size) {
  if(payload_size < 6) return;

  gyro_raw[0] = (payload[1] << 8) | payload[0];
  gyro_raw[1] = (payload[3] << 8) | payload[2];
  gyro_raw[2] = (payload[5] << 8) | payload[4];

  gyro_x = gyro_raw[0];
  gyro_y = gyro_raw[1];
  gyro_z = gyro_raw[2];
}

float imu::get_accel_x() {
  return accel_x;
}

float imu::get_accel_y() {
  return accel_y;
}

float imu::get_accel_z() {
  return accel_z;
}

float imu::get_gyro_x() {
  return gyro_x;
}

float imu::get_gyro_y() {
  return gyro_y;
}

float imu::get_gyro_z() {
  return gyro_z;
}
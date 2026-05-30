#include "imu.hpp"

imu::imu(void) {

}

void imu::init(void) {

}

void imu::update(void) {

}

void imu::update_accel(const uint8_t* data, uint8_t len) {
  if(len < 6) return;

  accel_raw[0] = (data[1] << 8) | data[0];
  accel_raw[1] = (data[3] << 8) | data[2];
  accel_raw[2] = (data[5] << 8) | data[4];

  accel_x = accel_raw[0];
  accel_y = accel_raw[1];
  accel_z = accel_raw[2];
}

void imu::update_gyro(const uint8_t* data, uint8_t len) {
  if(len < 6) return;

  gyro_raw[0] = (data[1] << 8) | data[0];
  gyro_raw[1] = (data[3] << 8) | data[2];
  gyro_raw[2] = (data[5] << 8) | data[4];

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
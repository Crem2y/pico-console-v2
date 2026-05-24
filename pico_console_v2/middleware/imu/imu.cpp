#include "imu.hpp"

imu::imu(void) {

}

void imu::init(void) {

}

void imu::update(void) {

}

void imu::update_from_bridge(uint8_t* data, uint8_t len) {
  if(len < 12) return;

  accel_raw[0] = (data[0] << 8) | data[1];
  accel_raw[1] = (data[2] << 8) | data[3];
  accel_raw[2] = (data[4] << 8) | data[5];
  gyro_raw[0] = (data[6] << 8) | data[7];
  gyro_raw[1] = (data[8] << 8) | data[9];
  gyro_raw[2] = (data[10] << 8) | data[11];

  accel_x = (int16_t)accel_raw[0];
  accel_y = (int16_t)accel_raw[1];
  accel_z = (int16_t)accel_raw[2];
  gyro_x = (int16_t)gyro_raw[0];
  gyro_y = (int16_t)gyro_raw[1];
  gyro_z = (int16_t)gyro_raw[2];
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
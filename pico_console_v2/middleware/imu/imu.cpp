#include "imu.hpp"
#include "bridge_protocol.hpp"

extern bridgeProtocol Bridge;

imu::imu(void) {

}

void imu::init(void) {

}

void imu::update(void) {

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

void imu::send_bridge_enable(bool enable_accel, bool enable_gyro) {
  int payload_size = 1;
  uint8_t payload_buf[PAYLOAD_MAX_SIZE];

  payload_buf[0] = (enable_accel ? 1 : 0) | (enable_gyro ? 2 : 0);

  Bridge.bridge_msg_push(CMD_IMU_ENABLE, 1, payload_buf);
}

void imu::send_bridge_disable(void) {
  Bridge.bridge_msg_push(CMD_IMU_DISABLE, 0, NULL);
}

void imu::recv_bridge_accel_data(const uint8_t* payload, uint8_t payload_size) {
  if(payload_size < 6) return;

  accel_raw[0] = (payload[1] << 8) | payload[0];
  accel_raw[1] = (payload[3] << 8) | payload[2];
  accel_raw[2] = (payload[5] << 8) | payload[4];

  //placeholder
  accel_x = (float)accel_raw[0];
  accel_y = (float)accel_raw[1];
  accel_z = (float)accel_raw[2];
}

void imu::recv_bridge_gyro_data(const uint8_t* payload, uint8_t payload_size) {
  if(payload_size < 6) return;

  gyro_raw[0] = (payload[1] << 8) | payload[0];
  gyro_raw[1] = (payload[3] << 8) | payload[2];
  gyro_raw[2] = (payload[5] << 8) | payload[4];

  //placeholder
  gyro_x = (float)gyro_raw[0];
  gyro_y = (float)gyro_raw[1];
  gyro_z = (float)gyro_raw[2];
}
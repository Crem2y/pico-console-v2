#include "imu.hpp"
#include "bridge_protocol.hpp"

extern bridgeProtocol Bridge;

imu::imu(mpu6050* mpu) {
  Mpu6050 = mpu;
}

void imu::init(void) {
  Mpu6050->init();

  enable_accel_data(false);
  enable_gyro_data(false);
}

void imu::update(void) {
  if(enable_accel) {
    Mpu6050->read_accel_data();

    accel_data_raw[0] = Mpu6050->accel_raw.x;
    accel_data_raw[1] = Mpu6050->accel_raw.y;
    accel_data_raw[2] = Mpu6050->accel_raw.z;

    send_bridge_accel_data();
  }

  if(enable_gyro) {
    Mpu6050->read_gyro_data();

    gyro_data_raw[0] = Mpu6050->gyro_raw.x;
    gyro_data_raw[1] = Mpu6050->gyro_raw.y;
    gyro_data_raw[2] = Mpu6050->gyro_raw.z;

    send_bridge_gyro_data();
  }
}

void imu::enable_accel_data(bool enable) {
  enable_accel = enable;
}

void imu::enable_gyro_data(bool enable) {
  enable_gyro = enable;
}

void imu::send_bridge_accel_data(void) {
  int payload_size = 6;
  uint8_t payload_buf[PAYLOAD_MAX_SIZE];

  //placeholder
  payload_buf[0] = accel_data_raw[0] & 0xFF;
  payload_buf[1] = accel_data_raw[0] >> 8;
  payload_buf[2] = accel_data_raw[1] & 0xFF;
  payload_buf[3] = accel_data_raw[1] >> 8;
  payload_buf[4] = accel_data_raw[2] & 0xFF;
  payload_buf[5] = accel_data_raw[2] >> 8;

  Bridge.send(CMD_IMU_ACCEL_DATA, payload_size, payload_buf);
}

void imu::send_bridge_gyro_data(void) {
  int payload_size = 6;
  uint8_t payload_buf[PAYLOAD_MAX_SIZE];

  //placeholder
  payload_buf[0] = gyro_data_raw[0] & 0xFF;
  payload_buf[1] = gyro_data_raw[0] >> 8;
  payload_buf[2] = gyro_data_raw[1] & 0xFF;
  payload_buf[3] = gyro_data_raw[1] >> 8;
  payload_buf[4] = gyro_data_raw[2] & 0xFF;
  payload_buf[5] = gyro_data_raw[2] >> 8;

  Bridge.send(CMD_IMU_GYRO_DATA, payload_size, payload_buf);
}

void imu::recv_bridge_enable(const uint8_t* payload, uint8_t payload_size) {
  if(payload_size < 1) return;

  bool enable_accel = (payload[0] & 0x01);
  bool enable_gyro = (payload[0] & 0x02);

  enable_accel_data(enable_accel);
  enable_gyro_data(enable_gyro);
}
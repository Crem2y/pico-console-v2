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

    accel_data[0] = Mpu6050->accel.x;
    accel_data[1] = Mpu6050->accel.y;
    accel_data[2] = Mpu6050->accel.z;

    send_bridge_accel_data();
  }

  if(enable_gyro) {
    Mpu6050->read_gyro_data();

    gyro_data[0] = Mpu6050->gyro.x;
    gyro_data[1] = Mpu6050->gyro.y;
    gyro_data[2] = Mpu6050->gyro.z;

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
  int payload_size = 12;
  uint8_t payload_buf[PAYLOAD_MAX_SIZE];

  memcpy(&payload_buf[0], &accel_data[0], sizeof(float));
  memcpy(&payload_buf[4], &accel_data[1], sizeof(float));
  memcpy(&payload_buf[8], &accel_data[2], sizeof(float));

  Bridge.send(CMD_IMU_ACCEL_DATA, payload_size, payload_buf);
}

void imu::send_bridge_gyro_data(void) {
  int payload_size = 12;
  uint8_t payload_buf[PAYLOAD_MAX_SIZE];

  memcpy(&payload_buf[0], &gyro_data[0], sizeof(float));
  memcpy(&payload_buf[4], &gyro_data[1], sizeof(float));
  memcpy(&payload_buf[8], &gyro_data[2], sizeof(float));

  Bridge.send(CMD_IMU_GYRO_DATA, payload_size, payload_buf);
}

void imu::recv_bridge_enable(const uint8_t* payload, uint8_t payload_size) {
  if(payload_size < 1) return;

  bool enable_accel = (payload[0] & 0x01);
  bool enable_gyro = (payload[0] & 0x02);

  enable_accel_data(enable_accel);
  enable_gyro_data(enable_gyro);
}
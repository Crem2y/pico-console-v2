#pragma once

enum bridge_cmd {
  CMD_NONE              = 0x00,

  CMD_TEMPERATURE_DATA  = 0x11,
  CMD_POWER_STATUS      = 0x12,

  CMD_GAMEPAD_DATA      = 0x21, // [btn1][btn2][joyLx][joyLy][joyRx][joyRy]

  CMD_IR_RX_ENABLE      = 0x30, // [format]
  CMD_IR_RX_DATA        = 0x31, // [format][data1][data2]...
  CMD_IR_RX_DISABLE     = 0x3F,

  CMD_IR_TX_ENABLE      = 0x40, // [format]
  CMD_IR_TX_DATA        = 0x41, // [format][data1][data2]...
  CMD_IR_TX_DISABLE     = 0x4F,

  CMD_IMU_ENABLE        = 0x50,
  CMD_IMU_DATA          = 0x51, // [ax][ay][az][gx][gy][gz][mx][my][mz]
  CMD_IMU_ACCEL_DATA    = 0x52,
  CMD_IMU_GYRO_DATA     = 0x53,
  CMD_IMU_MAG_DATA      = 0x54,
  CMD_IMU_DISABLE       = 0x5F,

  CMD_AUDIO_ENABLE      = 0x60,
  CMD_AUDIO_PCM_DATA    = 0x61,
  CMD_AUDIO_DISABLE     = 0x6F,

  CMD_VIBRATION_ENABLE  = 0x70,
  CMD_VIBRATION_DATA    = 0x71, // [Lfreql][Lfreqh][Lpwr][Rfreql][Rfreqh][Rpwr]
  CMD_VIBRATION_DISABLE = 0x7F,

  CMD_UNKNOWN           = 0xFF
};
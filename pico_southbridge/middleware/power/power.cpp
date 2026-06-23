#include "power.hpp"
#include "bridge_protocol.hpp"

extern bridgeProtocol Bridge;

power::power(AdcVSense* vbat) {
  Vin = vbat;
}

void power::init(void) {
  Vin->init();
}

void power::update(void) {
  input_voltage = Vin->read_voltage();

  send_bridge_power_status();
}

void power::send_bridge_power_status(void) {
  int payload_size = 2;
  uint8_t payload_buf[PAYLOAD_MAX_SIZE];

  uint16_t voltage_mv = (input_voltage * 1000);

  payload_buf[0] = voltage_mv & 0xFF;
  payload_buf[1] = voltage_mv >> 8;

  Bridge.send(CMD_POWER_STATUS, payload_size, payload_buf);
}
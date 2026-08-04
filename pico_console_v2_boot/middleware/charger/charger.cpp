#include "charger.hpp"
#include "bridge_protocol.hpp"

extern bridgeProtocol Bridge;

charger::charger(void) {

}

void charger::init(void) {
  voltage = 0.0f;
  level = 0.0f;

  is_battery_present = false;
  is_charging = false;
  is_external_power_present = false;

  fault = 0x00;

  charge_enable = true;
  max_charge_current = 500;
}

void charger::enable_charge(bool enable) {
  if(is_battery_present) {
    charge_enable = enable;
  } else {
    charge_enable = false;
  }
  send_bridge_bat_control(charge_enable, max_charge_current);
}

void charger::set_max_charge_current(uint16_t charge_ma) {
  if(charge_ma > 1500) charge_ma = 1500;

  max_charge_current = charge_ma;
  send_bridge_bat_control(charge_enable, max_charge_current);
}

void charger::send_bridge_bat_control(bool enable, uint16_t charge_ma) {
  int payload_size = 2;
  uint8_t payload_buf[PAYLOAD_MAX_SIZE];

  uint8_t charge_en = (enable ? 0x80 : 0x00);
  uint8_t current_ma = charge_ma / 10;

  payload_buf[0] = charge_en;
  payload_buf[1] = current_ma;

  Bridge.send(CMD_BATTERY_CONTROL, payload_size, payload_buf);
}

void charger::recv_bridge_bat_status(const uint8_t* payload, uint8_t payload_size) {
  if(payload_size < 6) return;

  uint16_t voltage_mv = (payload[1] << 8) | payload[0];
  voltage = (float)voltage_mv / 1000;

  uint16_t level_x10 = (payload[3] << 8) | payload[2];
  level = (float)level_x10 / 10;

  uint8_t status_flag = payload[4];
  is_battery_present = (status_flag & 0x80);
  is_charging = (status_flag & 0x40);
  is_external_power_present = (status_flag & 0x20);

  fault = payload[5];
}
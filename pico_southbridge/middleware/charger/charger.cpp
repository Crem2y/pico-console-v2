#include "charger.hpp"
#include "bridge_protocol.hpp"

extern bridgeProtocol Bridge;

charger::charger(liBattery* bat, bq25619* bq25619) {
  Bat = bat;
  Bq25619 = bq25619;
}

void charger::init(void) {
  Bat->init();
  Bq25619->init();

  charging = false;
  fault = 0x00;

  float voltage = Bat->get_voltage();
  if(voltage < 2.0f) {
    is_battery_exist = false;
    Bq25619->enable_charge(false);
  } else {
    is_battery_exist = true;
    Bq25619->set_ignore_ts(true);
  }
}

void charger::update(void) {
  Bat->get_level();

  Bq25619->update_watchdog();

  Bq25619->read_reg(BQ25619_REG_CHARGER_STATUS_0);
  charging = Bq25619->get_charging_status();

  Bq25619->read_reg(BQ25619_REG_CHARGER_STATUS_1);
  fault = Bq25619->get_fault_status();

  send_bridge_bat_status();
}

float charger::get_bat_voltage(void) {
  return Bat->voltage;
}

float charger::get_bat_level(void) {
  return Bat->level;
}

bool charger::get_charging_status(void) {
  return charging;
}

uint8_t charger::get_fault_status(void) {
  return fault;
}

void charger::send_bridge_bat_status(void) {
  int payload_size = 6;
  uint8_t payload_buf[PAYLOAD_MAX_SIZE];

  uint16_t voltage_mv = (Bat->voltage * 1000);
  uint16_t level_x10 = (Bat->level * 10);
  uint8_t status_flag = (is_battery_exist ? 0x80 : 0x00) | (charging ? 0x40 : 0x00);

  payload_buf[0] = voltage_mv & 0xFF;
  payload_buf[1] = voltage_mv >> 8;
  payload_buf[2] = level_x10 & 0xFF;
  payload_buf[3] = level_x10 >> 8;
  payload_buf[4] = status_flag;
  payload_buf[5] = fault;

  Bridge.send(CMD_BATTERY_STATUS, payload_size, payload_buf);
}

void charger::recv_bridge_bat_control(const uint8_t* payload, uint8_t payload_size) {

}
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
  if(voltage < 0.5f) {
    is_battery_exist = false;
  } else {
    is_battery_exist = true;
  }
}

void charger::update(void) {
  Bat->get_voltage();

  Bq25619->update_watchdog();

  Bq25619->read_reg(BQ25619_REG_CHARGER_STATUS_0);
  charging = Bq25619->get_charging_status();

  Bq25619->read_reg(BQ25619_REG_CHARGER_STATUS_1);
  fault = Bq25619->get_fault_status();

  send_bridge_bat_status();
}

float charger::get_bat_voltage(void) {
  if(is_battery_exist) {
    return 0.0f;
  } else {
    return Bat->voltage;
  }
}

float charger::get_bat_level(void) {
  if(is_battery_exist) {
    return 0.0f;
  } else {
    return Bat->level;
  }
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

  payload_buf[0] = voltage_mv & 0xFF;
  payload_buf[1] = voltage >> 8;
  payload_buf[2] = level_x10 & 0xFF;
  payload_buf[3] = level_x10 >> 8;
  payload_buf[4] = charging; //test
  payload_buf[5] = fault;

  Bridge.send(CMD_BATTERY_STATUS, payload_size, payload_buf);
}

void charger::recv_bridge_bat_control(const uint8_t* payload, uint8_t payload_size) {

}
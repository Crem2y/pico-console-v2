#include "charger.hpp"
#include "bridge_protocol.hpp"

extern bridgeProtocol Bridge;

charger::charger(AdcVSense* vbat, bq25619* bq25619) {
  Vbat = vbat;
  Bq25619 = bq25619;
}

void charger::init(void) {
  Vbat->init();
  Bq25619->init();

  charging = false;
  fault = 0x00;

  sleep_ms(100);

  bat_voltage = Vbat->read_voltage();
  if(bat_voltage < 2.0f) {
    is_battery_exist = false;
    Bq25619->enable_charge(false);
  } else {
    is_battery_exist = true;
    Bq25619->set_ignore_ts(true);
  }
}

void charger::update(void) {
  bat_voltage = Vbat->read_voltage();
  if(bat_voltage >= 4.2) {
    bat_level = 100;
  } else if(bat_voltage > 4.1) { // 99.9 ~ 96.0, 100mV
    bat_level = (((bat_voltage - 4.1) / 0.1) * 4) + 96;
  } else if(bat_voltage > 3.8) { // 95.9 ~ 56.0, 300mV
    bat_level = (((bat_voltage - 3.8) / 0.3) * 40) + 56;
  } else if(bat_voltage > 3.4) { // 55.9 ~ 13.0, 400mV
    bat_level = (((bat_voltage - 3.4) / 0.4) * 43) + 13;
  } else if(bat_voltage > 3.0) { // 12.9 ~ 0.0, 400mV
    bat_level = (((bat_voltage - 3.0) / 0.4) * 13) + 0;
  } else {
    bat_level = 0;
  }

  Bq25619->update_watchdog();

  Bq25619->read_reg(BQ25619_REG_CHARGER_STATUS_0);
  charging = Bq25619->get_charging_status();

  Bq25619->read_reg(BQ25619_REG_CHARGER_STATUS_1);
  fault = Bq25619->get_fault_status();

  send_bridge_bat_status();
}

void charger::send_bridge_bat_status(void) {
  int payload_size = 6;
  uint8_t payload_buf[PAYLOAD_MAX_SIZE];

  uint16_t voltage_mv = (bat_voltage * 1000);
  uint16_t level_x10 = (bat_level * 10);
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
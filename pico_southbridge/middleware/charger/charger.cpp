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

  is_charging = false;
  is_external_power_present = false;

  fault = 0x00;

  sleep_ms(100);

  bat_voltage = Vbat->read_voltage();
  if(bat_voltage < 2.0f) {
    is_battery_present = false;
    Bq25619->enable_charge(false);
  } else {
    is_battery_present = true;
    Bq25619->set_ignore_ts(true);
  }

  Bq25619->set_input_current_limit(500);

  Bq25619->read_reg(BQ25619_REG_CHARGE_CURRENT_LIMIT);
  LOGI("before ichg : %dmA\n", Bq25619->get_charge_current());

  Bq25619->set_charge_current(500);

  Bq25619->read_reg(BQ25619_REG_CHARGE_CURRENT_LIMIT);
  LOGI("after ichg : %dmA\n", Bq25619->get_charge_current());
}

void charger::update(void) {
  Bq25619->update_watchdog();

  Bq25619->read_reg(BQ25619_REG_CHARGER_STATUS_0);
  is_charging = Bq25619->get_charging_status();
  is_external_power_present = Bq25619->get_external_power_status();

  Bq25619->read_reg(BQ25619_REG_CHARGER_STATUS_1);
  fault = Bq25619->get_fault_status();

  float bat_voltage_raw = Vbat->read_voltage();

  if(!is_external_power_present) {
    bat_voltage = bat_voltage_raw + 0.080; // 80mV
  } else if(is_charging) {
    bat_voltage = bat_voltage_raw - 0.080; // 80mV
  } else {
    bat_voltage = bat_voltage_raw;
  }

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

  send_bridge_bat_status();
}

void charger::send_bridge_bat_status(void) {
  int payload_size = 6;
  uint8_t payload_buf[PAYLOAD_MAX_SIZE];

  uint16_t voltage_mv = (bat_voltage * 1000);
  uint16_t level_x10 = (bat_level * 10);
  uint8_t status_flag = (is_battery_present ? 0x80 : 0x00) | (is_charging ? 0x40 : 0x00) | (is_external_power_present ? 0x20 : 0x00);

  payload_buf[0] = voltage_mv & 0xFF;
  payload_buf[1] = voltage_mv >> 8;
  payload_buf[2] = level_x10 & 0xFF;
  payload_buf[3] = level_x10 >> 8;
  payload_buf[4] = status_flag;
  payload_buf[5] = fault;

  Bridge.send(CMD_BATTERY_STATUS, payload_size, payload_buf);
}

void charger::recv_bridge_bat_control(const uint8_t* payload, uint8_t payload_size) {
  if(payload_size < 2) return;

  bool charge_en = payload[0];
  uint16_t current_ma = payload[1] * 10;

  enable_charge(charge_en);
  set_max_charge_current(current_ma);
}
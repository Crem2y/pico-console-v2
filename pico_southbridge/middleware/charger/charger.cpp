#include "charger.hpp"
#include "bridge_protocol.hpp"

extern bridgeProtocol Bridge;

charger::charger(bq25619* bq25619) {
  Bq25619 = bq25619;
}

void charger::init(void) {
  Bq25619->init();

  Bq25619->set_ignore_ts(true);
}

void charger::update(void) {
  Bq25619->update_watchdog();

  Bq25619->read_reg(BQ25619_REG_CHARGER_STATUS_0);
  charging = Bq25619->get_charging_status();

  Bq25619->read_reg(BQ25619_REG_CHARGER_STATUS_1);
  fault = Bq25619->get_fault_status();
}
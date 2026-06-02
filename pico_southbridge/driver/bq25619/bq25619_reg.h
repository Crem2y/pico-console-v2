#pragma once

#include <stdint.h>

#define BQ25619_REG_INPUT_CURRENT_LIMIT     0x00
#define BQ25619_REG_CHARGER_CONTROL_0       0x01
#define BQ25619_REG_CHARGE_CURRENT_LIMIT    0x02
#define BQ25619_REG_PRECHARGE_CURRENT_LIMIT 0x03
#define BQ25619_REG_BATTERY_VOLTAGE_LIMIT   0x04
#define BQ25619_REG_CHARGER_CONTROL_1       0x05
#define BQ25619_REG_CHARGER_CONTROL_2       0x06
#define BQ25619_REG_CHARGER_CONTROL_3       0x07
#define BQ25619_REG_CHARGER_STATUS_0        0x08
#define BQ25619_REG_CHARGER_STATUS_1        0x09
#define BQ25619_REG_CHARGER_STATUS_2        0x0A
#define BQ25619_REG_PART_INFORMATION        0x0B
#define BQ25619_REG_CHARGER_CONTROL_4       0x0C

union bq25619_input_current_limit_reg {
  uint8_t byte;
  struct {
    uint8_t IINDPM : 5;
    uint8_t BATSNS_DIS : 1;
    uint8_t TS_IGNORE : 1;
    uint8_t EN_HIZ : 1;
  };
};

union bq25619_charger_ctrl_0_reg {
  uint8_t byte;
  struct {
    uint8_t MIN_VBAT_SEL : 1;
    uint8_t SYS_MIN : 3;
    uint8_t CHG_CONFIG : 1;
    uint8_t BST_CONFIG : 1;
    uint8_t WD_RST : 1;
    uint8_t PFM_DIS : 1;
  };
};

union bq25619_charge_current_limit_reg {
  uint8_t byte;
  struct {
    uint8_t ICHG : 6;
    uint8_t Q1_FULLON : 1;
    uint8_t reserved : 1;
  };
};

union bq25619_precharge_current_limit_reg {
  uint8_t byte;
  struct {
    uint8_t ITERM : 4;
    uint8_t IPRECHG : 4;
  };
};

union bq25619_battery_voltage_limit_reg {
  uint8_t byte;
  struct {
    uint8_t VRECHG : 1;
    uint8_t TOPOFF_TIMER : 2;
    uint8_t VBATREG : 5;
  };
};

union bq25619_charger_ctrl_1_reg {
  uint8_t byte;
  struct {
    uint8_t JEITA_VSET : 1;
    uint8_t TREG : 1;
    uint8_t CHG_TIMER : 1;
    uint8_t EN_TIMER : 1;
    uint8_t WATCHDOG : 2;
    uint8_t reserved : 1;
    uint8_t EN_TERM : 1;
  };
};

union bq25619_charger_ctrl_2_reg {
  uint8_t byte;
  struct {
    uint8_t VINDPM : 4;
    uint8_t BOOSTV : 2;
    uint8_t OVP : 2;
  };
};

union bq25619_charger_ctrl_3_reg {
  uint8_t byte;
  struct {
    uint8_t VINDPM_BAT_TRACK : 2;
    uint8_t BATFET_RST_EN : 1;
    uint8_t BATFET_DLY : 1;
    uint8_t BATFET_RST_WVBUS : 1;
    uint8_t BATFET_DIS : 1;
    uint8_t TMR2X_EN : 1;
    uint8_t IINDET_EN : 1;
  };
};

union bq25619_charger_status_0_reg {
  uint8_t byte;
  struct {
    uint8_t VSYS_STAT : 1;
    uint8_t THERM_STAT : 1;
    uint8_t PG_STAT : 1;
    uint8_t CHRG_STAT : 2;
    uint8_t VBUS_STAT : 3;
  };
};

union bq25619_charger_status_1_reg {
  uint8_t byte;
  struct {
    uint8_t NTC_FAULT : 3;
    uint8_t BAT_FAULT : 1;
    uint8_t CHRG_FAULT : 2;
    uint8_t BOOST_FAULT : 1;
    uint8_t WATCHDOG_FAULT : 1;
  };
};

union bq25619_charger_status_2_reg {
  uint8_t byte;
  struct {
    uint8_t IINDPM_INT_MASK : 1;
    uint8_t VINDPM_INT_MASK : 1;
    uint8_t ACOV_STAT : 1;
    uint8_t TOPOFF_ACTIVE : 1;
    uint8_t BATSNS_STAT : 1;
    uint8_t IINDPM_STAT : 1;
    uint8_t VINDPM_STAT : 1;
    uint8_t VBUS_GD : 1;
  };
};

union bq25619_part_info_reg {
  uint8_t byte;
  struct {
    uint8_t reserved : 3;
    uint8_t PN : 4;
    uint8_t REG_RST : 1;
  };
};

union bq25619_charger_ctrl_4_reg {
  uint8_t byte;
  struct {
    uint8_t JEITA_VT3 : 4;
    uint8_t JEITA_WARM_ISET : 2;
    uint8_t JEITA_COOL_ISET : 2;
  };
};
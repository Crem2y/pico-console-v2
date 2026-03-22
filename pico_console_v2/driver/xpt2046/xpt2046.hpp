#pragma once

#include "pico/stdlib.h"
#include "hardware/spi.h"

#define XPT2046_SPI_SPEED (1*1000*1000)

#define XPT2046_ADDR_X    0xD0
#define XPT2046_ADDR_Y    0x90
#define XPT2046_ADDR_Z1   0xB0
#define XPT2046_ADDR_Z2   0xC0

#define XPT2046_ADDR_TEMP 0xF0
#define XPT2046_ADDR_VBAT 0xA0
#define XPT2046_ADDR_AUX  0xE0

#define XPT2046_MODE_12   0x00
#define XPT2046_MODE_08   0x08

#define XPT2046_REF_SING  0x04
#define XPT2046_REF_DIFF  0x00

typedef struct _touch_data_t {
  uint16_t x;
  uint16_t y;
  uint16_t z1;
  uint16_t z2;
} touch_data_t;

class xpt2046 {
  public:
    xpt2046(spi_inst_t* spi, int pin_tx, int pin_rx, int pin_sck, int pin_cs, int pin_irq);
    void init(void);

    uint16_t get_x(void);
    uint16_t get_y(void);
    uint16_t get_z1(void);
    uint16_t get_z2(void);

    void get_touch_data(void);

    touch_data_t touch_data;

  private:
    spi_inst_t* _spi;
    int _pin_tx, _pin_rx, _pin_sck;
    int _pin_cs, _pin_irq;
};
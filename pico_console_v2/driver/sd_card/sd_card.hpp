#pragma once

#include <stdint.h>
#include "uart_log.h"

// sd lib (carlk3/no-OS-FatFS-SD-SDIO-SPI-RPi-Pico)
#include "f_util.h"
#include "hw_config.h"
#include "sd_card.h"
#include "diskio.h" /* Declarations of disk functions */

typedef int (*printer_wrapper_t)(const char* format, ...);

class sdCard {
  public:
    sdCard(void);

    void init(void);
    void update(void);

    void mount(void);

    void print_info(printer_wrapper_t printer);

    bool is_inserted(void);
    bool is_mounted(void);

  private:
    sd_card_t *sd_card_p;
};

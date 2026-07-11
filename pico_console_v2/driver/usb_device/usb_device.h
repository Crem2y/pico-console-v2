#pragma once

#include <stdint.h>
#include "uart_log.h"

#include "bsp/board.h"
#include "tusb.h"
#include "usb_descriptors.h"

#ifdef __cplusplus
extern "C" {
#endif

void usbDevice_init(void);
void usbDevice_update(void);
void usbDevice_update_10ms(void);

void usbDevice_update_keyboard(bool const keys_pressed, const uint8_t* key_codes);

#ifdef __cplusplus
}
#endif
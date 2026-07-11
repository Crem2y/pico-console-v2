#include "usb_device.h"

uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t *buffer, uint16_t reqlen) {
  // not Implemented
  (void)instance;
  (void)report_id;
  (void)report_type;
  (void)buffer;
  (void)reqlen;

  return 0;
}

void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const *buffer, uint16_t bufsize) {
  (void)instance;

  if (report_type == HID_REPORT_TYPE_OUTPUT) {
    // Set keyboard LED e.g Capslock, Numlock etc...
    if (report_id == REPORT_ID_KEYBOARD) {
      // bufsize should be (at least) 1
      if (bufsize < 1)
        return;

      uint8_t const kbd_leds = buffer[0];
    }
  }
}

static void send_hid_report(bool keys_pressed, const uint8_t* key_codes) {
  // skip if hid is not ready yet
  if (!tud_hid_ready()) {
    return;
  }

  // avoid sending multiple zero reports
  static bool send_empty = false;

  if (keys_pressed) {
    tud_hid_keyboard_report(REPORT_ID_KEYBOARD, 0, key_codes);
    send_empty = true;
  } else {
    // send empty key report if previously has key pressed
    if (send_empty) {
      tud_hid_keyboard_report(REPORT_ID_KEYBOARD, 0, NULL);
    }
    send_empty = false;
  }
}

void hid_task(bool const keys_pressed, const uint8_t* key_codes) {

  // Remote wakeup
  if (tud_suspended() && keys_pressed)
  {
    // Wake up host if we are in suspend mode
    // and REMOTE_WAKEUP feature is enabled by host
    tud_remote_wakeup();
  }
  else
  {
    // send a report
    send_hid_report(keys_pressed, key_codes);
  }
}

void usbDevice_init(void) {
  tusb_init();
}
void usbDevice_update(void) {
  tud_task();
}

void usbDevice_update_10ms(bool const keys_pressed, const uint8_t* key_codes) {
  hid_task(keys_pressed, key_codes);
}
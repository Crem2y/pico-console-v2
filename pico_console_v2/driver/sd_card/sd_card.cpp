#include "sd_card.hpp"

static volatile bool card_det_int_pend;
static volatile uint card_det_int_gpio;

static void process_card_detect_int() {
  card_det_int_pend = false;
  for (size_t i = 0; i < sd_get_num(); ++i) {
    sd_card_t *sd_card_p = sd_get_by_num(i);
    if (!sd_card_p)
      continue;
    if (sd_card_p->card_detect_gpio == card_det_int_gpio) {
      if (sd_card_p->state.mounted) {
        LOGW("(Card Detect Interrupt: unmounting %s)\n", sd_get_drive_prefix(sd_card_p));
        FRESULT fr = f_unmount(sd_get_drive_prefix(sd_card_p));
        if (FR_OK == fr) {
          sd_card_p->state.mounted = false;
        } else {
          LOGE("f_unmount error: %s (%d)\n", FRESULT_str(fr), fr);
        }
      }
      sd_card_p->state.m_Status |= STA_NOINIT;  // in case medium is removed
      sd_card_detect(sd_card_p);
    }
  }
}

static void card_detect_callback(uint gpio, uint32_t events) {
  (void)events;
  // This is actually an interrupt service routine!
  card_det_int_gpio = gpio;
  card_det_int_pend = true;
}

sdCard::sdCard(void) {

}

void sdCard::init(void) {
  sd_init_driver();
  sd_card_p = sd_get_by_num(0);
  if (sd_card_p->use_card_detect) {
    gpio_set_irq_enabled_with_callback(
      sd_card_p->card_detect_gpio, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL,
      true, &card_detect_callback);
  }
}

void sdCard::update(void) {
  if (card_det_int_pend) {
    process_card_detect_int();
  }
}

void sdCard::mount(void) {
  if (!sd_card_p) return;

  int ds = sd_card_p->init(sd_card_p);
  if (STA_NODISK & ds || STA_NOINIT & ds) {
    LOGW("SD card initialization failed\n");
  }

  size_t au_size_bytes;
  bool ok = sd_allocation_unit(sd_card_p, &au_size_bytes);

  FATFS *fs_p = &sd_card_p->state.fatfs;
  FRESULT fr = f_mount(fs_p, sd_get_drive_prefix(sd_get_by_num(0)), 1);
  if (FR_OK != fr) {
      LOGE("f_mount error: %s (%d)\n", FRESULT_str(fr), fr);
  }
  sd_card_p->state.mounted = true;
}

bool sdCard::is_inserted(void) {
  if (!sd_card_p) return false;
  return !(STA_NODISK & sd_card_p->state.m_Status);
}

bool sdCard::is_mounted(void) {
  if (!sd_card_p) return false;
  return sd_card_p->state.mounted;
}

void sdCard::print_info(printer_wrapper_t printer) {
  if (!sd_card_p) return;

  cidDmp(sd_card_p, printer);
  csdDmp(sd_card_p, printer);
}
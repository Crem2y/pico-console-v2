// headers
#include "common.h"
#include "hw_test.hpp"
#include "v2_hw_def.h"

#include "pico/bootrom.h"
#include "hardware/flash.h"
#include "hardware/regs/addressmap.h"
#include "boot/picobin.h"

#define BOOTROM_WORKAREA_SIZE 4096

uint8_t boot_partition_num = 0;
static uint8_t workarea[BOOTROM_WORKAREA_SIZE] __attribute__((aligned(4)));

int boot_partition(uint partition_num) {
    uint32_t info[3];

    int rc = rom_load_partition_table(
        workarea,
        sizeof(workarea),
        false
    );

    LOGI("load PT rc = %d\n", rc);

    if (rc < 0)
        return rc;

    rc = rom_get_partition_table_info(
        info,
        count_of(info),
        PT_INFO_SINGLE_PARTITION |
        PT_INFO_PARTITION_LOCATION_AND_FLAGS |
        (partition_num << 24)
    );

    LOGI("get PT info rc = %d\n", rc);

    if (rc < 0)
        return rc;

    LOGI("supported flags = 0x%08lx\n", info[0]);
    LOGI("perm/location   = 0x%08lx\n", info[1]);
    LOGI("perm/flags      = 0x%08lx\n", info[2]);

    if (!(info[0] & PT_INFO_PARTITION_LOCATION_AND_FLAGS)) {
        LOGE("location info not returned\n");
        return BOOTROM_ERROR_INVALID_DATA;
    }

    const uint32_t location = info[1];

    const uint32_t first_sector =
        (location & PICOBIN_PARTITION_LOCATION_FIRST_SECTOR_BITS)
        >> PICOBIN_PARTITION_LOCATION_FIRST_SECTOR_LSB;

    const uint32_t last_sector =
        (location & PICOBIN_PARTITION_LOCATION_LAST_SECTOR_BITS)
        >> PICOBIN_PARTITION_LOCATION_LAST_SECTOR_LSB;

    const uint32_t flash_offset =
        first_sector * FLASH_SECTOR_SIZE;

    const uint32_t flash_size =
        (last_sector - first_sector + 1) * FLASH_SECTOR_SIZE;

    const uint32_t region_base =
        XIP_BASE + flash_offset;

    LOGI("P%u sector = %lu .. %lu\n",
           partition_num,
           first_sector,
           last_sector);

    LOGI("region = 0x%08lx .. 0x%08lx (%lu bytes)\n",
           region_base,
           region_base + flash_size,
           flash_size);

    /*
     * IMAGE_DEF / block loop가 실제로 이 영역 첫 4 KiB에 있는지
     * 확인하기 위한 간단한 dump.
     */
    const uint32_t *p = (const uint32_t *)region_base;

    for (uint i = 0; i < 0x1000 / 4; ++i) {
        if (p[i] == PICOBIN_BLOCK_MARKER_START) {
            LOGI("PICOBIN block found at +0x%03x\n", i * 4);
        }
    }

    LOGI("chain...\n");

    sleep_ms(1000);
    multicore_reset_core1();

    rc = rom_chain_image(
        workarea,
        sizeof(workarea),
        region_base,
        flash_size
    );

    LOGE("chain failed: %d\n", rc);

    return rc;
}

// hw lib init
ledStatus Led = ledStatus(PIN_LED_WL_1, PIN_LED_WL_2, PIN_LED_WL_3, PIN_LED_WL_4);
ili9488_40 Lcd = ili9488_40(PIN_DP_MOSI, PIN_DP_SCK, PIN_DP_CS, PIN_DP_DC, PIN_DP_RST, PIN_DP_BL);
xpt2046 Touch = xpt2046(HW_TOUCH_CH, PIN_TOUCH_MOSI, PIN_TOUCH_SCK, PIN_TOUCH_MISO, PIN_TOUCH_CS, PIN_TOUCH_IRQ);
sdCard Sd = sdCard();
pio_uart_tx_t pio_tx;
pio_uart_rx_t pio_rx;
#if ENABLE_RFBRIDGE
pio_uart_tx_t pio_tx_rf;
pio_uart_rx_t pio_rx_rf;
#endif

// middleware lib init
bridgeProtocol Bridge = bridgeProtocol();
bridgeControl SouthBridge = bridgeControl(&Bridge);
#if ENABLE_RFBRIDGE
bridgeProtocol BridgeRf = bridgeProtocol();
bridgeControl RfBridge = bridgeControl(&BridgeRf);
#endif
power Power = power();
charger Charger = charger();
#if !ENABLE_HW_LED && ENABLE_SW_LED
ledControl LedCtrl = ledControl(&BridgeRf);
#else
ledControl LedCtrl = ledControl(&Led);
#endif
gamepad Gamepad = gamepad();
graphicSystem Graphic = graphicSystem(&Lcd);

void core1_entry();
void bridge_cmd_handler(const bridge_msg_t* msg);
#if ENABLE_RFBRIDGE
void bridge_cmd_handler_rf(const bridge_msg_t* msg);
#endif

time_ms_t bridge_timer;
time_ms_t led_timer;
time_ms_t gamepad_timer;
time_ms_t sd_timer;

inline int pio_uart_readable_wrapper(void) {
  return pio_uart_rx_readable(&pio_rx);
}
inline int pio_uart_read_wrapper(uint8_t* data, size_t buf_size) {
  return pio_uart_rx_read(&pio_rx, data, buf_size);
}
inline int pio_uart_writeable_wrapper(void) {
  return pio_uart_tx_writeable(&pio_tx);
}
inline int pio_uart_write_wrapper(const uint8_t* data, size_t data_size) {
  return pio_uart_tx_write(&pio_tx, data, data_size);
}

#if ENABLE_RFBRIDGE
inline int pio_uart_readable_wrapper_rf(void) {
  return pio_uart_rx_readable(&pio_rx_rf);
}
inline int pio_uart_read_wrapper_rf(uint8_t* data, size_t buf_size) {
  return pio_uart_rx_read(&pio_rx_rf, data, buf_size);
}
inline int pio_uart_writeable_wrapper_rf(void) {
  return pio_uart_tx_writeable(&pio_tx_rf);
}
inline int pio_uart_write_wrapper_rf(const uint8_t* data, size_t data_size) {
  return pio_uart_tx_write(&pio_tx_rf, data, data_size);
}
#endif
//////// function ////////

int main() { // uses core 0 to sub core
  // log init
  uartLog_init(HW_LOG_CH, PIN_LOG_TX, PIN_LOG_RX, HW_LOG_BAUD);
  uartLog_print("\n\npico console V2 booting...\n\n");

  // bridge init
  pio_uart_tx_init(&pio_tx, HW_BRIDGE_PIO, PIN_BRIDGE_TX, HW_BRIDGE_BAUD);
  pio_uart_rx_init(&pio_rx, HW_BRIDGE_PIO, PIN_BRIDGE_RX, HW_BRIDGE_BAUD);
  bridge_transport_t transport = {pio_uart_readable_wrapper, pio_uart_read_wrapper, pio_uart_writeable_wrapper, pio_uart_write_wrapper};
  Bridge.set_transport_handler(&transport);
  Bridge.set_cmd_handler(bridge_cmd_handler);
  SouthBridge.init();

#if ENABLE_RFBRIDGE // rf bridge init
  pio_uart_tx_init(&pio_tx_rf, HW_RF_BRIDGE_PIO, PIN_RF_BRIDGE_TX, HW_RF_BRIDGE_BAUD);
  pio_uart_rx_init(&pio_rx_rf, HW_RF_BRIDGE_PIO, PIN_RF_BRIDGE_RX, HW_RF_BRIDGE_BAUD);
  bridge_transport_t transport_rf = {pio_uart_readable_wrapper_rf, pio_uart_read_wrapper_rf, pio_uart_writeable_wrapper_rf, pio_uart_write_wrapper_rf};
  BridgeRf.set_transport_handler(&transport_rf);
  BridgeRf.set_cmd_handler(bridge_cmd_handler_rf);
  RfBridge.init();
#endif
  sleep_ms(100);

  LedCtrl.init();
  led_config_t led_config = {.mode = LED_BLINK_REPEAT, .brightness = 255, .update_interval_ms = 500};
  LedCtrl.set_config(LED_CTRL_BUILT_IN, led_config);
  LedCtrl.update();

  // initalizing hardwares
  Power.init();
  Charger.init();
  led_config = {.mode = LED_ON, .brightness = 255, .update_interval_ms = 20, .breathing_step = 10};
  LedCtrl.set_config(LED_CTRL_1, led_config);
  LedCtrl.set_config(LED_CTRL_2, led_config);
  LedCtrl.set_config(LED_CTRL_3, led_config);
  LedCtrl.set_config(LED_CTRL_4, led_config);
  LedCtrl.update();
  LOGI("LED ok\n");
#if ENABLE_PSRAM
  int32_t ret = psram_init(PIN_PSRAM_CS);
  if(ret < 0) {
    LOGE("PSRAM error : %d", ret);
    while(1);
  }
  LOGI("PSRAM ok\n");
#endif
  Graphic.begin();
  Graphic.fillScreen(LCD_BLACK);
  Graphic.set_bright(750);
  Graphic.setTextColor(LCD_WHITE, LCD_BLACK);
  Graphic.setTextSize(1);
  Graphic.set_font(G_FONT_5X8);
  LOGI("LCD ok\n");
  Graphic.setCursor(0,0);
  Graphic.print("Gamepad init...");
  Gamepad.init();
  Gamepad.set_enable(true, false);
  LOGI("Gamepad ok\n");
  Graphic.setCursor(0,0);
  Graphic.print("SD init...");
  Sd.init();
  LOGI("SD ok\n");
  Graphic.setCursor(0,0);
  Graphic.print("               ");
  LOGI("all HWs ok!\n");
  LOGI("core freq = %ld hz\n", SYS_CLK_KHZ * 1000);
  // hardware initalized

  LOGI("go to main loop\n");
  multicore_launch_core1(core1_entry);
  // multicore_fifo_push_blocking(1);
  // boot sequence end

  while (true) {
    time_ms_t now_time = get_system_time_ms();

    Bridge.process_io();
    Bridge.dispatch_rx();
#if ENABLE_RFBRIDGE
    BridgeRf.process_io();
    BridgeRf.dispatch_rx();
#endif

    if(system_time_elapsed_ms(now_time, bridge_timer) > 1000) {
      bridge_timer = now_time;
      SouthBridge.update();
#if ENABLE_RFBRIDGE
      RfBridge.update();
#endif
    }
    if(system_time_elapsed_ms(now_time, led_timer) > 10) {
      led_timer = now_time;
      LedCtrl.update();
    }
    if(system_time_elapsed_ms(now_time, gamepad_timer) > 10) {
      gamepad_timer = now_time;
      Gamepad.update();
    }
    if(system_time_elapsed_ms(now_time, sd_timer) > 10) {
      sd_timer = now_time;
      Sd.update();
    }
    if(boot_partition_num) {
      boot_partition(boot_partition_num);
      boot_partition_num = 0;
    }
  }

  return 0;
}

void core1_entry() { // uses core 1 to main core

  // multicore_fifo_pop_blocking(); // wait until boot process is done

  Graphic.setTextSize(2);
  Graphic.setCursor(150,160);
  Graphic.print("PICO BOOTLOADER");

  Graphic.setCursor(480-(6*2*9),320-(8*2));
  Graphic.print("by Crem2y");
  Graphic.setTextSize(1);
  Graphic.setCursor(206,200);
  Graphic.print("press START");
  Graphic.setCursor(183,210);
  Graphic.print("or touch the screen");

  Graphic.setCursor(0,0);
  Graphic.print("press L/R to change bright");

  time_ms_t btn_time_ms = 0;
  time_ms_t display_time_ms = 0;
  bool display_text = false;
  bool display_bridge_status = false;
  while(true) {
    time_ms_t now_time = get_system_time_ms();
    if(Gamepad.is_btn_pressed(BTN_START)) break;

    if(system_time_elapsed_ms(now_time, btn_time_ms) > 200) {
      btn_time_ms = now_time;

      uint16_t bright = Graphic.get_bright();
      if(Gamepad.is_btn_pressed(BTN_SL) && bright > 50) {
        Graphic.set_bright(bright - 50);
        Graphic.setCursor(0,8);
        Graphic.printf("bright : %d ", bright - 50);
      }
      if(Gamepad.is_btn_pressed(BTN_SR) && bright < 1000) {
        Graphic.set_bright(bright + 50);
        Graphic.setCursor(0,8);
        Graphic.printf("bright : %d ", bright + 50);
      }

      Graphic.setCursor(480-66,0);
      Graphic.printf("BAT:% 3.1f%%", Charger.get_bat_level());
    }

    if(system_time_elapsed_ms(now_time, display_time_ms) > 1000) {
      display_time_ms = now_time;
      if(display_text) {
        Graphic.fillRect(206,200,(6*11),8,LCD_BLACK);
      } else {
        Graphic.setCursor(206,200);
        Graphic.print("press START");
      }
      display_text = !display_text;
    }

    // to remove flickering
    if(!SouthBridge.connected) {
      if(!display_bridge_status) {
        Graphic.setCursor(162,240);
        Graphic.print("southbridge disconnected!");
        display_bridge_status = true;
      }
    } else {
      if(display_bridge_status) {
        Graphic.fillRect(162,240,(26*6),8,LCD_BLACK);
        display_bridge_status = false;
      }
    }
  }

  LedCtrl.set_mode(LED_CTRL_1, LED_DARKER);
  LedCtrl.set_mode(LED_CTRL_2, LED_DARKER);
  LedCtrl.set_mode(LED_CTRL_3, LED_DARKER);
  LedCtrl.set_mode(LED_CTRL_4, LED_DARKER);

  uint8_t cursor_x = 0;
  uint8_t cursor_x_old = 0;

main_menu_loop:
  Graphic.fillScreen(LCD_BLACK);
  Graphic.setTextColor(LCD_WHITE, LCD_BLACK);

  while (1) {
    Graphic.setTextSize(2);
    Graphic.setCursor(0,0);
    Graphic.print(" System info\n");
    Graphic.print(" Boot to 1\n");

    Graphic.setTextSize(1);
    Graphic.setCursor(0,320-(8*2));
    Graphic.print("press up/down to move cursor");
    Graphic.setCursor(0,320-(8*1));
    Graphic.print("press A or START to select");

    while(1) {
      sleep_ms(100);

      Graphic.setTextSize(1);
      Graphic.setCursor(480-66,0);
      Graphic.printf("BAT:% 3.1f%%", Charger.get_bat_level());

      Graphic.setTextSize(2);
      if(cursor_x_old != cursor_x) {
        Graphic.setCursor(0,cursor_x_old * 16);
        Graphic.print(" ");
      }
      Graphic.setCursor(0,cursor_x * 16);
      Graphic.print("-");

      cursor_x_old = cursor_x;

      if(Gamepad.is_btn_pressed(BTN_S1_UP) || Gamepad.is_btn_pressed(BTN_UP)) {
        if(cursor_x > 0) cursor_x--;
      }
      if(Gamepad.is_btn_pressed(BTN_S1_DOWN) || Gamepad.is_btn_pressed(BTN_DOWN)) {
        if(cursor_x < MAIN_BOOT_1) cursor_x++;
      }

      if(!Gamepad.is_btn_pressed(BTN_SELECT) && (Gamepad.is_btn_pressed(BTN_A) || Gamepad.is_btn_pressed(BTN_START))) {
        Graphic.fillScreen(LCD_BLACK);
        switch (cursor_x)
        {
        case MAIN_SYSTEM_INFO:
          menu_system_info();
          break;
        case MAIN_BOOT_1:
          boot_partition_num = 1;
          break;
        default:
          cursor_x = 0;
          break;
        }
        goto main_menu_loop;
      }
    }
  }
}

//////// test menus ////////

void menu_system_info(void) {
  Graphic.setTextSize(1);
  Graphic.setCursor(0,320-8);
  Graphic.print("press SELECT & START to exit menu");
  Graphic.setTextSize(2);
  Graphic.setCursor(0,0);
  Graphic.print("System info");

  Graphic.setCursor(480-(6*2*9),320-(8*2));
  Graphic.print("by Crem2y");

  while(1) {
    sleep_ms(100);

    Graphic.setCursor(0,16*2);
    Graphic.setTextSize(1);
    Graphic.printf("SW version(MAIN): 0x%04X\n", SW_INFO_VERSION);
    Graphic.printf("build date      : %02d%02d%02d %02d%02d%02d\n\n", DATE_YY, DATE_MM, DATE_DD, TIME_HH, TIME_MM, TIME_SS);

    Graphic.printf("HW name         : %s\n", SouthBridge.info.hw_name);
    Graphic.printf("HW version      : 0x%04X\n", SouthBridge.info.hw_ver);
    Graphic.printf("HW support_flag : 0x%08X\n\n", SouthBridge.info.hw_support);

    Graphic.printf("SB Connected    : %s\n", SouthBridge.connected ? "Yes" : "No ");
    Graphic.printf("SW version (SB) : 0x%04X\n", SouthBridge.info.sw_ver);
    Graphic.printf("build date      : %06d %06d\n", SouthBridge.info.build_date, SouthBridge.info.build_time);
    Graphic.printf("SW support_flag : 0x%08X\n\n", SouthBridge.info.sw_support);
#if ENABLE_RFBRIDGE
    Graphic.printf("RF Connected    : %s\n", RfBridge.connected ? "Yes" : "No ");
    Graphic.printf("RF Module name  : %s\n", RfBridge.info.hw_name);
    Graphic.printf("SW version (RF) : 0x%04X\n", RfBridge.info.sw_ver);
    Graphic.printf("build date      : %06d %06d\n", RfBridge.info.build_date, RfBridge.info.build_time);
    Graphic.printf("SW support_flag : 0x%08X\n\n", RfBridge.info.sw_support);
#endif

    SouthBridge.read_hw_info();
    SouthBridge.read_hw_name();
    SouthBridge.read_sw_info();
#if ENABLE_RFBRIDGE
    RfBridge.read_hw_info();
    RfBridge.read_hw_name();
    RfBridge.read_sw_info();
#endif

    if(Gamepad.is_btn_pressed(BTN_SELECT) && Gamepad.is_btn_pressed(BTN_START)) {
      return;
    }
  }
}

void bridge_cmd_handler(const bridge_msg_t* msg) {
  enum bridge_cmd command = (enum bridge_cmd)msg->cmd;
  SouthBridge.update_last_comm_time();

  switch (command)
  {
  case CMD_HW_INFO_RES:
    SouthBridge.recv_bridge_hw_info_res(msg->payload, msg->payload_size);
    break;
  case CMD_HW_NAME_RES:
    SouthBridge.recv_bridge_hw_name_res(msg->payload, msg->payload_size);
    break;
  case CMD_SW_INFO_RES:
    SouthBridge.recv_bridge_sw_info_res(msg->payload, msg->payload_size);
    break;
  case CMD_POWER_STATUS:
    Power.recv_bridge_power_status(msg->payload, msg->payload_size);
    break;
  case CMD_BATTERY_STATUS:
    Charger.recv_bridge_bat_status(msg->payload, msg->payload_size);
    break;
  case CMD_GAMEPAD_DATA:
    Gamepad.recv_bridge_data(msg->payload, msg->payload_size);
    break;
  default:
    break;
  }
}

#if ENABLE_RFBRIDGE
void bridge_cmd_handler_rf(const bridge_msg_t* msg) {
  enum bridge_cmd command = (enum bridge_cmd)msg->cmd;
  RfBridge.update_last_comm_time();

  switch (command)
  {
  case CMD_HW_INFO_RES:
    RfBridge.recv_bridge_hw_info_res(msg->payload, msg->payload_size);
    break;
  case CMD_HW_NAME_RES:
    RfBridge.recv_bridge_hw_name_res(msg->payload, msg->payload_size);
    break;
  case CMD_SW_INFO_RES:
    RfBridge.recv_bridge_sw_info_res(msg->payload, msg->payload_size);
    break;
  default:
    break;
  }
}
#endif
// headers
#include "hw_test.hpp"
#include "v2_hw_def.h"

// hw lib init
ledStatus Led = ledStatus(PIN_LED_WL_1, PIN_LED_WL_2, PIN_LED_WL_3, PIN_LED_WL_4);
ili9488_40 Lcd = ili9488_40(PIN_DP_MOSI, PIN_DP_SCK, PIN_DP_CS, PIN_DP_DC, PIN_DP_RST, PIN_DP_BL);
xpt2046 Touch = xpt2046(HW_TOUCH_CH, PIN_TOUCH_MOSI, PIN_TOUCH_SCK, PIN_TOUCH_MISO, PIN_TOUCH_CS, PIN_TOUCH_IRQ);

// middleware lib init
bridgeProtocol Bridge = bridgeProtocol();
power Power = power();
charger Charger = charger();
ledControl LedCtrl = ledControl(&Led);
gamepad Gamepad = gamepad();
audioSystem Audio = audioSystem();
temperature Temperature = temperature();
vibration Vibration = vibration();
touchscreen Touchscreen = touchscreen(&Touch);
imu Imu = imu();
irLink Ir = irLink();

void core1_entry();
void bridge_do_cmd(const bridge_msg_t* packet);

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
                DBG_PRINTF("(Card Detect Interrupt: unmounting %s)\n", sd_get_drive_prefix(sd_card_p));
                FRESULT fr = f_unmount(sd_get_drive_prefix(sd_card_p));
                if (FR_OK == fr) {
                    sd_card_p->state.mounted = false;
                } else {
                    printf("f_unmount error: %s (%d)\n", FRESULT_str(fr), fr);
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

time_ms_t last_bridge_cmd_time;
time_ms_t gamepad_timer;
time_ms_t temperature_timer;
time_ms_t audio_timer;
time_ms_t vibration_timer;
time_ms_t touch_timer;

//////// function ////////

int main() { // uses core 0 to sub core
  // uartLog_init(HW_LOG_CH, PIN_LOG_TX, PIN_LOG_RX, HW_LOG_BAUD);
  uart_bridge_init(HW_BRIDGE_CH, PIN_BRIDGE_TX, PIN_BRIDGE_RX, HW_BRIDGE_BAUD);
  bridge_transport_t transport = {uart_bridge_readable, uart_bridge_read, uart_bridge_writable, uart_bridge_write};
  Bridge.set_transport_handler(&transport);
  Bridge.set_cmd_handler(bridge_do_cmd);
  last_bridge_cmd_time = 0;
  sleep_ms(100);

  LedCtrl.init();
  led_config_t led_config = {.mode = LED_BLINK_REPEAT, .brightness = 255, .blink_interval_ms = 500};
  LedCtrl.set_config(LED_CONTROL_BUILT_IN, led_config);
  LedCtrl.update();

  multicore_launch_core1(core1_entry);

  // initalizing hardwares
  Power.init();
  Charger.init();
  led_config = {.mode = LED_ON, .brightness = 255, .breathing_step = 10};
  LedCtrl.set_config(LED_CONTROL_1, led_config);
  LedCtrl.set_config(LED_CONTROL_2, led_config);
  LedCtrl.set_config(LED_CONTROL_3, led_config);
  LedCtrl.set_config(LED_CONTROL_4, led_config);
  LedCtrl.update();
  LOG_PRINTF("LED ok\n");
  Lcd.begin();
  Lcd.fillScreen(LCD_BLACK);
  Lcd.set_bright(750);
  Lcd.setTextColor(LCD_WHITE, LCD_BLACK);
  Lcd.setTextSize(1);
  LOG_PRINTF("LCD ok\n");
  Lcd.setCursor(0,0);
  Lcd.print_5x8("Touch init...");
  Touchscreen.init();
  Touchscreen.set_screen_size(480, 320);
  Touchscreen.set_calibration(200, 3800, 200, 3800); //placeholder
  Touchscreen.set_rotation(0);
  LOG_PRINTF("Touch ok\n");
  Lcd.setCursor(0,0);
  Lcd.print_5x8("Gamepad init...");
  Gamepad.init();
  Gamepad.set_enable(true, false);
  LOG_PRINTF("Gamepad ok\n");
  Lcd.setCursor(0,0);
  Lcd.print_5x8("TEMP init...");
  Temperature.init();
  LOG_PRINTF("TEMP ok\n");
  Lcd.setCursor(0,0);
  Lcd.print_5x8("LRA init...");
  Vibration.init();
  LOG_PRINTF("Vibration ok\n");
  Lcd.setCursor(0,0);
  Lcd.print_5x8("IMU init...");
  Imu.init();
  LOG_PRINTF("IMU ok\n");
  Lcd.setCursor(0,0);
  Lcd.print_5x8("IR init...");
  Ir.init();
  LOG_PRINTF("IR ok\n");
  Lcd.setCursor(0,0);
  Lcd.print_5x8("SD init...");
  sd_init_driver();
  sd_card_t *sd_card_p = sd_get_by_num(0);
  LOG_PRINTF("SD ok\n");
  Lcd.setCursor(0,0);
  Lcd.print_5x8("               ");
  LOG_PRINTF("all HWs ok!\n");
  LOG_PRINTF("core freq = %ld hz\n", SYS_CLK_KHZ * 1000);
  // hardware initalized

  uart_bridge_enable_irq();

  LOG_PRINTF("go to main loop\n");
  multicore_fifo_push_blocking(1);
  // boot sequence end

  while (true) {
    time_ms_t now_time = get_system_time_ms();

    Bridge.process_io();
    Bridge.dispatch_rx();

    if(system_time_elapsed_ms(now_time, gamepad_timer) > 10) {
      gamepad_timer = now_time;
      Gamepad.update();
    }
    if(system_time_elapsed_ms(now_time, temperature_timer) > 1000) {
      temperature_timer = now_time;
      Temperature.update();
    }
    if(system_time_elapsed_ms(now_time, audio_timer) > 1) {
      audio_timer = now_time;
      Audio.update();
    }
    if(system_time_elapsed_ms(now_time, vibration_timer) > 10) {
      vibration_timer = now_time;
      Vibration.update();
    }
    if(system_time_elapsed_ms(now_time, touch_timer) > 10) {
      touch_timer = now_time;
      Touchscreen.update();
      //LOG_PRINTF("x: %d, y: %d, z1: %d, z2: %d\n", Touchscreen.touch_data.x, Touchscreen.touch_data.y, Touchscreen.touch_data.z1, Touchscreen.touch_data.z2);
    }
    LedCtrl.update();
    if (card_det_int_pend) {
      process_card_detect_int();
    }
  }

  return 0;
}

void core1_entry() { // uses core 1 to main core

  multicore_fifo_pop_blocking(); // wait until boot process is done

  // boot animation
  Lcd.setTextSize(2);
  for(int i=0; i<160; i+=1) {
    Lcd.fillRect(140, i-1, (6*2*16), 1, LCD_BLACK);
    Lcd.setCursor(140,i);
    Lcd.print_5x8("PICO CONSOLE V2");
    sleep_ms(10);
  }

  Lcd.setCursor(480-(6*2*9),320-(8*2));
  Lcd.print_5x8("by Crem2y");
  Lcd.setTextSize(1);
  Lcd.setCursor(190,200);
  Lcd.print_5x8("press START");
  Lcd.setCursor(170,210);
  Lcd.print_5x8("or touch the screen");

  Lcd.setCursor(0,0);
  Lcd.print_5x8("press L/R to change bright");

  time_ms_t btn_time_ms = 0;
  time_ms_t display_time_ms = 0;
  bool display_text = false;
  bool display_bridge_status = false;
  while(true) {
    time_ms_t now_time = get_system_time_ms();
    if(Gamepad.is_btn_pressed(BTN_START)) break;
    if(Touchscreen.is_touched()) break;

    if(system_time_elapsed_ms(now_time, btn_time_ms) > 200) {
      btn_time_ms = now_time;
      char string_buf[32];

      uint16_t bright = Lcd.get_bright();
      if(Gamepad.is_btn_pressed(BTN_SL) && bright > 50) {
        Lcd.set_bright(bright - 50);
        sprintf(string_buf, "bright : %d ", bright - 50);
        Lcd.setCursor(0,8);
        Lcd.print_5x8(string_buf);
      }
      if(Gamepad.is_btn_pressed(BTN_SR) && bright < 1000) {
        Lcd.set_bright(bright + 50);
        sprintf(string_buf, "bright : %d ", bright + 50);
        Lcd.setCursor(0,8);
        Lcd.print_5x8(string_buf);
      }

      sprintf(string_buf, "BAT:% 3.1f%%", Charger.get_bat_level());
      Lcd.setCursor(480-66,0);
      Lcd.print_5x8(string_buf);
    }

    if(system_time_elapsed_ms(now_time, display_time_ms) > 1000) {
      display_time_ms = now_time;
      if(display_text) {
        Lcd.fillRect(190,200,66,8,LCD_BLACK);
      } else {
        Lcd.setCursor(190,200);
        Lcd.print_5x8("press START");
      }
      display_text = !display_text;
    }

    // to remove flickering
    if(system_time_elapsed_ms(get_system_time_ms(), last_bridge_cmd_time) > 500) {
      if(!display_bridge_status) {
        Lcd.setCursor(150,240);
        Lcd.print_5x8("southbridge disconnected!");
        display_bridge_status = true;
      }
    } else {
      if(display_bridge_status) {
        Lcd.fillRect(150,240,(26*6),8,LCD_BLACK);
        display_bridge_status = false;
      }
    }
  }

  LedCtrl.set_mode(LED_CONTROL_1, LED_DARKER);
  LedCtrl.set_mode(LED_CONTROL_2, LED_DARKER);
  LedCtrl.set_mode(LED_CONTROL_3, LED_DARKER);
  LedCtrl.set_mode(LED_CONTROL_4, LED_DARKER);

  music_note_t boot_notes[2] = {
    {0, 6, 0},   // C6
    {0, 7, 0}    // C7
  };

  music_table_t boot_music = {
    .len = 2,
    .note_duration_ms = 100,
    .notes = boot_notes
  };

  // if SELECT+START, quiet boot
  if(!Gamepad.is_btn_pressed(BTN_SELECT)) {
    Audio.play_music(&boot_music);
  }

  char string_buf[32];
  uint8_t cursor_x = 0;
  uint8_t cursor_x_old = 0;

main_menu_loop:
  Lcd.fillScreen(LCD_BLACK);
  Lcd.setTextColor(LCD_WHITE, LCD_BLACK);

  while (1) {
    Lcd.setTextSize(2);
    Lcd.setCursor(16,0);
    Lcd.print_5x8("Button test");
    Lcd.setCursor(16,16);
    Lcd.print_5x8("Joystick test");
    Lcd.setCursor(16,32);
    Lcd.print_5x8("LED test");
    Lcd.setCursor(16,48);
    Lcd.print_5x8("LCD test");
    Lcd.setCursor(16,64);
    Lcd.print_5x8("Touch test");
    Lcd.setCursor(16,80);
    Lcd.print_5x8("Audio test");
    Lcd.setCursor(16,96);
    Lcd.print_5x8("Vibration test");
    Lcd.setCursor(16,112);
    Lcd.print_5x8("Battery & Power test");
    Lcd.setCursor(16,128);
    Lcd.print_5x8("Temperature test");
    Lcd.setCursor(16,144);
    Lcd.print_5x8("IR comm test");
    Lcd.setCursor(16,160);
    Lcd.print_5x8("IMU test");
    Lcd.setCursor(16,176);
    Lcd.print_5x8("SD card test");

    Lcd.setTextSize(1);
    Lcd.setCursor(0,320-(8*2));
    Lcd.print_5x8("press up/down to move cursor");
    Lcd.setCursor(0,320-(8*1));
    Lcd.print_5x8("press A or START to select");

    while(1) {
      sleep_ms(100);

      Lcd.setTextSize(1);
      sprintf(string_buf, "BAT:% 3.1f%%", Charger.get_bat_level());
      Lcd.setCursor(480-66,0);
      Lcd.print_5x8(string_buf);

      Lcd.setTextSize(2);
      if(cursor_x_old != cursor_x) {
        Lcd.setCursor(0,cursor_x_old * 16);
        Lcd.print_5x8(" ");
      }
      Lcd.setCursor(0,cursor_x * 16);
      Lcd.print_5x8("-");

      cursor_x_old = cursor_x;

      if(Gamepad.is_btn_pressed(BTN_S1_UP) || Gamepad.is_btn_pressed(BTN_UP)) {
        if(cursor_x > 0) cursor_x--;
      }
      if(Gamepad.is_btn_pressed(BTN_S1_DOWN) || Gamepad.is_btn_pressed(BTN_DOWN)) {
        if(cursor_x < MAIN_SD_TEST) cursor_x++;
      }

      if(!Gamepad.is_btn_pressed(BTN_SELECT) && (Gamepad.is_btn_pressed(BTN_A) || Gamepad.is_btn_pressed(BTN_START))) {
        Lcd.fillScreen(LCD_BLACK);
        switch (cursor_x)
        {
        case MAIN_BTN_TEST:
          menu_btn_test();
          break;
        case MAIN_JOYSTICK_TEST:
          menu_joystick_test();
          break;
        case MAIN_LED_TEST:
          menu_led_test();
          break;
        case MAIN_LCD_TEST:
          menu_lcd_test();
          break;
        case MAIN_TOUCH_TEST:
          menu_touch_test();
          break;
        case MAIN_AUDIO_TEST:
          menu_audio_test();
          break;
        case MAIN_VIBRATION_TEST:
          menu_vibration_test();
          break;
        case MAIN_BAT_TEST:
          menu_bat_test();
          break;
        case MAIN_TEMP_TEST:
          menu_temp_test();
          break;
        case MAIN_IR_TEST:
          menu_ir_test();
          break;
        case MAIN_IMU_TEST:
          menu_imu_test();
          break;
        case MAIN_SD_TEST:
          menu_sd_test();
          break;
        }
        goto main_menu_loop;
      }
    }
  }
}

//////// test menus ////////

void menu_btn_test(void) {
  Lcd.setTextSize(1);
  Lcd.setCursor(0,320-8);
  Lcd.print_5x8("press SELECT & START to exit menu");
  Lcd.setTextSize(2);
  Lcd.setCursor(0,0);
  Lcd.print_5x8("Button test");
  
  Lcd.drawRect(20,64,280,156,LCD_WHITE);
  Lcd.drawRect(152,200,16,16,LCD_RED);

  while(1) {
    sleep_ms(10);
    
    if(Gamepad.is_btn_pressed(BTN_SL)) {
      if(Gamepad.get_btn_pressed_duration(BTN_SL) > 1000) {
        Lcd.fillRect(20,46,16,16,LCD_YELLOW);
      } else {
        Lcd.fillRect(20,46,16,16,LCD_WHITE);
      }
    } else {
      Lcd.fillRect(21,47,14,14,LCD_BLACK);
      Lcd.drawRect(20,46,16,16,LCD_WHITE);
    }
    if(Gamepad.is_btn_pressed(BTN_SR)) {
      if(Gamepad.get_btn_pressed_duration(BTN_SR) > 1000) {
        Lcd.fillRect(284,46,16,16,LCD_YELLOW);
      } else {
        Lcd.fillRect(284,46,16,16,LCD_WHITE);
      }
    } else {
      Lcd.fillRect(285,47,14,14,LCD_BLACK);
      Lcd.drawRect(284,46,16,16,LCD_WHITE);
    }
    if(Gamepad.is_btn_pressed(BTN_ZL)) {
      if(Gamepad.get_btn_pressed_duration(BTN_ZL) > 1000) {
        Lcd.fillRect(60,46,16,16,LCD_YELLOW);
      } else {
        Lcd.fillRect(60,46,16,16,LCD_WHITE);
      }
    } else {
      Lcd.fillRect(61,47,14,14,LCD_BLACK);
      Lcd.drawRect(60,46,16,16,LCD_WHITE);
    }
    if(Gamepad.is_btn_pressed(BTN_ZR)) {
      if(Gamepad.get_btn_pressed_duration(BTN_ZR) > 1000) {
        Lcd.fillRect(244,46,16,16,LCD_YELLOW);
      } else {
        Lcd.fillRect(244,46,16,16,LCD_WHITE);
      }
    } else {
      Lcd.fillRect(245,47,14,14,LCD_BLACK);
      Lcd.drawRect(244,46,16,16,LCD_WHITE);
    }
    if(Gamepad.is_btn_pressed(BTN_SELECT)) {
      if(Gamepad.get_btn_pressed_duration(BTN_SELECT) > 1000) {
        Lcd.fillRect(100,74,16,16,LCD_YELLOW);
      } else {
        Lcd.fillRect(100,74,16,16,LCD_WHITE);
      }
    } else {
      Lcd.fillRect(101,75,14,14,LCD_BLACK);
      Lcd.drawRect(100,74,16,16,LCD_WHITE);
    }
    if(Gamepad.is_btn_pressed(BTN_START)) {
      if(Gamepad.get_btn_pressed_duration(BTN_START) > 1000) {
        Lcd.fillRect(205,74,16,16,LCD_YELLOW);
      } else {
        Lcd.fillRect(205,74,16,16,LCD_WHITE);
      }
    } else {
      Lcd.fillRect(206,75,14,14,LCD_BLACK);
      Lcd.drawRect(205,74,16,16,LCD_WHITE);
    }

    if(Gamepad.is_btn_pressed(BTN_S1_CENTER)) {
      if(Gamepad.get_btn_pressed_duration(BTN_S1_CENTER) > 1000) {
        Lcd.fillRect(50,94,16,16,LCD_YELLOW);
      } else {
        Lcd.fillRect(50,94,16,16,LCD_WHITE);
      }
    } else {
      Lcd.fillRect(51,95,14,14,LCD_BLACK);
      Lcd.drawRect(50,94,16,16,LCD_WHITE);
    }
    if(Gamepad.is_btn_pressed(BTN_S1_UP)) {
      if(Gamepad.get_btn_pressed_duration(BTN_S1_UP) > 1000) {
        Lcd.fillRect(50,74,16,16,LCD_YELLOW);
      } else {
        Lcd.fillRect(50,74,16,16,LCD_WHITE);
      }
    } else {
      Lcd.fillRect(51,75,14,14,LCD_BLACK);
      Lcd.drawRect(50,74,16,16,LCD_WHITE);
    }
    if(Gamepad.is_btn_pressed(BTN_S1_DOWN)) {
      if(Gamepad.get_btn_pressed_duration(BTN_S1_DOWN) > 1000) {
        Lcd.fillRect(50,114,16,16,LCD_YELLOW);
      } else {
        Lcd.fillRect(50,114,16,16,LCD_WHITE);
      }
    } else {
      Lcd.fillRect(51,115,14,14,LCD_BLACK);
      Lcd.drawRect(50,114,16,16,LCD_WHITE);
    }
    if(Gamepad.is_btn_pressed(BTN_S1_LEFT)) {
      if(Gamepad.get_btn_pressed_duration(BTN_S1_LEFT) > 1000) {
        Lcd.fillRect(30,94,16,16,LCD_YELLOW);
      } else {
        Lcd.fillRect(30,94,16,16,LCD_WHITE);
      }
    } else {
      Lcd.fillRect(31,95,14,14,LCD_BLACK);
      Lcd.drawRect(30,94,16,16,LCD_WHITE);
    }
    if(Gamepad.is_btn_pressed(BTN_S1_RIGHT)) {
      if(Gamepad.get_btn_pressed_duration(BTN_S1_RIGHT) > 1000) {
        Lcd.fillRect(70,94,16,16,LCD_YELLOW);
      } else {
        Lcd.fillRect(70,94,16,16,LCD_WHITE);
      }
    } else {
      Lcd.fillRect(71,95,14,14,LCD_BLACK);
      Lcd.drawRect(70,94,16,16,LCD_WHITE);
    }

    if(Gamepad.is_btn_pressed(BTN_UP)) {
      if(Gamepad.get_btn_pressed_duration(BTN_UP) > 1000) {
        Lcd.fillRect(50,154,16,16,LCD_YELLOW);
      } else {
        Lcd.fillRect(50,154,16,16,LCD_WHITE);
      }
    } else {
      Lcd.fillRect(51,155,14,14,LCD_BLACK);
      Lcd.drawRect(50,154,16,16,LCD_WHITE);
    }
    if(Gamepad.is_btn_pressed(BTN_DOWN)) {
      if(Gamepad.get_btn_pressed_duration(BTN_DOWN) > 1000) {
        Lcd.fillRect(50,194,16,16,LCD_YELLOW);
      } else {
        Lcd.fillRect(50,194,16,16,LCD_WHITE);
      }
    } else {
      Lcd.fillRect(51,195,14,14,LCD_BLACK);
      Lcd.drawRect(50,194,16,16,LCD_WHITE);
    }
    if(Gamepad.is_btn_pressed(BTN_LEFT)) {
      if(Gamepad.get_btn_pressed_duration(BTN_LEFT) > 1000) {
        Lcd.fillRect(30,174,16,16,LCD_YELLOW);
      } else {
        Lcd.fillRect(30,174,16,16,LCD_WHITE);
      }
    } else {
      Lcd.fillRect(31,175,14,14,LCD_BLACK);
      Lcd.drawRect(30,174,16,16,LCD_WHITE);
    }
    if(Gamepad.is_btn_pressed(BTN_RIGHT)) {
      if(Gamepad.get_btn_pressed_duration(BTN_RIGHT) > 1000) {
        Lcd.fillRect(70,174,16,16,LCD_YELLOW);
      } else {
        Lcd.fillRect(70,174,16,16,LCD_WHITE);
      }
    } else {
      Lcd.fillRect(71,175,14,14,LCD_BLACK);
      Lcd.drawRect(70,174,16,16,LCD_WHITE);
    }

    if(Gamepad.is_btn_pressed(BTN_S2_CENTER)) {
      if(Gamepad.get_btn_pressed_duration(BTN_S2_CENTER) > 1000) {
        Lcd.fillRect(255,174,16,16,LCD_YELLOW);
      } else {
        Lcd.fillRect(255,174,16,16,LCD_WHITE);
      }
    } else {
      Lcd.fillRect(256,175,14,14,LCD_BLACK);
      Lcd.drawRect(255,174,16,16,LCD_WHITE);
    }
    if(Gamepad.is_btn_pressed(BTN_X)) {
      if(Gamepad.get_btn_pressed_duration(BTN_X) > 1000) {
        Lcd.fillRect(255,74,16,16,LCD_YELLOW);
      } else {
        Lcd.fillRect(255,74,16,16,LCD_WHITE);
      }
    } else {
      Lcd.fillRect(256,75,14,14,LCD_BLACK);
      Lcd.drawRect(255,74,16,16,LCD_WHITE);
    }
    if(Gamepad.is_btn_pressed(BTN_B)) {
      if(Gamepad.get_btn_pressed_duration(BTN_B) > 1000) {
        Lcd.fillRect(255,114,16,16,LCD_YELLOW);
      } else {
        Lcd.fillRect(255,114,16,16,LCD_WHITE);
      }
    } else {
      Lcd.fillRect(256,115,14,14,LCD_BLACK);
      Lcd.drawRect(255,114,16,16,LCD_WHITE);
    }
    if(Gamepad.is_btn_pressed(BTN_Y)) {
      if(Gamepad.get_btn_pressed_duration(BTN_Y) > 1000) {
        Lcd.fillRect(235,94,16,16,LCD_YELLOW);
      } else {
        Lcd.fillRect(235,94,16,16,LCD_WHITE);
      }
    } else {
      Lcd.fillRect(236,95,14,14,LCD_BLACK);
      Lcd.drawRect(235,94,16,16,LCD_WHITE);
    }
    if(Gamepad.is_btn_pressed(BTN_A)) {
      if(Gamepad.get_btn_pressed_duration(BTN_A) > 1000) {
        Lcd.fillRect(274,94,16,16,LCD_YELLOW);
      } else {
        Lcd.fillRect(274,94,16,16,LCD_WHITE);
      }
    } else {
      Lcd.fillRect(275,95,14,14,LCD_BLACK);
      Lcd.drawRect(274,94,16,16,LCD_WHITE);
    }
        if(Gamepad.is_btn_pressed(BTN_SUB1)) {
      if(Gamepad.get_btn_pressed_duration(BTN_SUB1) > 1000) {
        Lcd.fillRect(100,194,16,16,LCD_YELLOW);
      } else {
        Lcd.fillRect(100,194,16,16,LCD_WHITE);
      }
    } else {
      Lcd.fillRect(101,195,14,14,LCD_BLACK);
      Lcd.drawRect(100,194,16,16,LCD_WHITE);
    }
    if(Gamepad.is_btn_pressed(BTN_SUB2)) {
      if(Gamepad.get_btn_pressed_duration(BTN_SUB2) > 1000) {
        Lcd.fillRect(205,194,16,16,LCD_YELLOW);
      } else {
        Lcd.fillRect(205,194,16,16,LCD_WHITE);
      }
    } else {
      Lcd.fillRect(206,195,14,14,LCD_BLACK);
      Lcd.drawRect(205,194,16,16,LCD_WHITE);
    }

    if(Gamepad.is_btn_pressed(BTN_S2_UP)) {
      if(Gamepad.get_btn_pressed_duration(BTN_S2_UP) > 1000) {
        Lcd.fillRect(255,154,16,16,LCD_YELLOW);
      } else {
        Lcd.fillRect(255,154,16,16,LCD_WHITE);
      }
    } else {
      Lcd.fillRect(256,155,14,14,LCD_BLACK);
      Lcd.drawRect(255,154,16,16,LCD_WHITE);
    }
    if(Gamepad.is_btn_pressed(BTN_S2_DOWN)) {
      if(Gamepad.get_btn_pressed_duration(BTN_S2_DOWN) > 1000) {
        Lcd.fillRect(255,194,16,16,LCD_YELLOW);
      } else {
        Lcd.fillRect(255,194,16,16,LCD_WHITE);
      }
    } else {
      Lcd.fillRect(256,195,14,14,LCD_BLACK);
      Lcd.drawRect(255,194,16,16,LCD_WHITE);
    }
    if(Gamepad.is_btn_pressed(BTN_S2_LEFT)) {
      if(Gamepad.get_btn_pressed_duration(BTN_S2_LEFT) > 1000) {
        Lcd.fillRect(235,174,16,16,LCD_YELLOW);
      } else {
        Lcd.fillRect(235,174,16,16,LCD_WHITE);
      }
    } else {
      Lcd.fillRect(236,175,14,14,LCD_BLACK);
      Lcd.drawRect(235,174,16,16,LCD_WHITE);
    }
    if(Gamepad.is_btn_pressed(BTN_S2_RIGHT)) {
      if(Gamepad.get_btn_pressed_duration(BTN_S2_RIGHT) > 1000) {
        Lcd.fillRect(274,174,16,16,LCD_YELLOW);
      } else {
        Lcd.fillRect(274,174,16,16,LCD_WHITE);
      }
    } else {
      Lcd.fillRect(275,175,14,14,LCD_BLACK);
      Lcd.drawRect(274,174,16,16,LCD_WHITE);
    }


    if(Gamepad.is_btn_pressed(BTN_SELECT) && Gamepad.is_btn_pressed(BTN_START)) {
      return;
    }
  }
}

void menu_joystick_test(void) {
  Lcd.setTextSize(1);
  Lcd.setCursor(0,320-8);
  Lcd.print_5x8("press SELECT & START to exit menu");
  Lcd.setTextSize(2);
  Lcd.setCursor(0,0);
  Lcd.print_5x8("Joystick test");

  uint16_t prev_pos1_x, prev_pos1_y;
  uint16_t prev_pos2_x, prev_pos2_y;

  Lcd.drawRect(160-72, 160-72, 128+15, 128+15, LCD_WHITE);
  Lcd.drawRect(320-72, 160-72, 128+15, 128+15, LCD_WHITE);

  Gamepad.set_enable(true, true);

  while(1) {
    sleep_ms(10);
    char string_buf[32];

    int8_t joy1_x = Gamepad.get_joystick_data(JOY1_X);
    int8_t joy1_y = Gamepad.get_joystick_data(JOY1_Y);

    uint16_t pos1_x = 160 + (joy1_x >> 1);
    uint16_t pos1_y = 160 + (joy1_y >> 1);

    if(pos1_x != prev_pos1_x || pos1_y != prev_pos1_y) {
      Lcd.fillRect(prev_pos1_x-7,prev_pos1_y-7,14,14,LCD_BLACK);
      Lcd.drawCircle(159,159,71,LCD_WHITE);
      Lcd.drawLine(88,160,88+128+15,160,LCD_WHITE);
      Lcd.drawLine(160,88,160,88+128+15,LCD_WHITE);
      Lcd.fillRect(pos1_x-7,pos1_y-7,14,14,LCD_WHITE);
      prev_pos1_x = pos1_x;
      prev_pos1_y = pos1_y;

      sprintf(string_buf, "L stick : % 4d, % 4d", joy1_x, joy1_y);
      Lcd.setCursor(0,16);
      Lcd.print_5x8(string_buf);
      sprintf(string_buf, "L raw   :% 5d,% 5d", Gamepad.get_joystick_raw(JOY1_X), Gamepad.get_joystick_raw(JOY1_Y));
      Lcd.setCursor(0,16*2);
      Lcd.print_5x8(string_buf);
    }

    int8_t joy2_x = Gamepad.get_joystick_data(JOY2_X);
    int8_t joy2_y = Gamepad.get_joystick_data(JOY2_Y);

    uint16_t pos2_x = 320 + (joy2_x >> 1);
    uint16_t pos2_y = 160 + (joy2_y >> 1);

    if(pos2_x != prev_pos2_x || pos2_y != prev_pos2_y) {
      Lcd.fillRect(prev_pos2_x-7,prev_pos2_y-7,14,14,LCD_BLACK);
      Lcd.drawCircle(319,159,71,LCD_WHITE);
      Lcd.drawLine(248,160,248+128+15,160,LCD_WHITE);
      Lcd.drawLine(320,88,320,88+128+15,LCD_WHITE);
      Lcd.fillRect(pos2_x-7,pos2_y-7,14,14,LCD_WHITE);
      prev_pos2_x = pos2_x;
      prev_pos2_y = pos2_y;

      sprintf(string_buf, "R stick : % 4d, % 4d", joy2_x, joy2_y);
      Lcd.setCursor(0,16*3);
      Lcd.print_5x8(string_buf);
      sprintf(string_buf, "R raw   :% 5d,% 5d", Gamepad.get_joystick_raw(JOY2_X), Gamepad.get_joystick_raw(JOY2_Y));
      Lcd.setCursor(0,16*4);
      Lcd.print_5x8(string_buf);
    }

    if(Gamepad.is_btn_pressed(BTN_SELECT) && Gamepad.is_btn_pressed(BTN_START)) {
      Gamepad.set_enable(true, false);
      return;
    }
  }
}

void menu_led_test(void) {
  Lcd.setTextSize(1);
  Lcd.setCursor(0,320-8);
  Lcd.print_5x8("press SELECT & START to exit menu");
  Lcd.setTextSize(2);
  Lcd.setCursor(0,0);
  Lcd.print_5x8("LED test");

  int now_mode = LED_OFF;
  const char* led_mode_string[] = {
    "LED_OFF             ",
    "LED_ON              ",
    "LED_BLINK_ONCE      ",
    "LED_BLINK_REPEAT    ",
    "LED_BRIGHTER        ",
    "LED_DARKER          ",
    "LED_BREATHING_ONCE  ",
    "LED_BREATHING_REPEAT",
  };

  led_config_t led_config = {.mode = (enum led_ctrl_mode)now_mode, .brightness = 255, .blink_interval_ms = 500, .breathing_step = 5};
  LedCtrl.set_config(LED_CONTROL_1, led_config);
  LedCtrl.set_config(LED_CONTROL_2, led_config);
  LedCtrl.set_config(LED_CONTROL_3, led_config);
  LedCtrl.set_config(LED_CONTROL_4, led_config);

  while(1) {
    sleep_ms(100);

    char string_buf[32];

    if(Gamepad.get_btn_released_duration(BTN_START) && Gamepad.get_btn_released_duration(BTN_START) < 100) {
      now_mode++;
      if(now_mode > LED_BREATHING_REPEAT) now_mode = (int)LED_OFF;

      LedCtrl.set_mode(LED_CONTROL_1, (enum led_ctrl_mode)now_mode);
      LedCtrl.set_mode(LED_CONTROL_2, (enum led_ctrl_mode)now_mode);
      LedCtrl.set_mode(LED_CONTROL_3, (enum led_ctrl_mode)now_mode);
      LedCtrl.set_mode(LED_CONTROL_4, (enum led_ctrl_mode)now_mode);
    }

    sprintf(string_buf, "LED mode : %s", led_mode_string[now_mode]);
    Lcd.setCursor(0,16);
    Lcd.print_5x8(string_buf);

    if(Gamepad.is_btn_pressed(BTN_SELECT) && Gamepad.is_btn_pressed(BTN_START)) {
      LedCtrl.set_mode(LED_CONTROL_1, LED_OFF);
      LedCtrl.set_mode(LED_CONTROL_2, LED_OFF);
      LedCtrl.set_mode(LED_CONTROL_3, LED_OFF);
      LedCtrl.set_mode(LED_CONTROL_4, LED_OFF);
      return;
    }
  }
}

void menu_lcd_test(void) {
  Lcd.setTextSize(1);
  Lcd.setCursor(0,320-8);
  Lcd.print_5x8("press SELECT & START to exit menu");
  Lcd.setTextSize(2);
  Lcd.setCursor(0,0);
  Lcd.print_5x8("LCD test");

  int count = 0;
  uint16_t color = 0x0000;

  while(1) {
    sleep_ms(500);

    char string_buf[32];
    time_us_t start_time = get_system_time_us();

    switch(count) {
      case 0:
        Lcd.fillRect(0,16,480,(320-24),LCD_WHITE);
        break;
      case 1:
        Lcd.fillRect(0,16,480,(320-24),LCD_RED);
        break;
      case 2:
        Lcd.fillRect(0,16,480,(320-24),LCD_GREEN);
        break;
      case 3:
        Lcd.fillRect(0,16,480,(320-24),LCD_BLUE);
        break;
      case 4:
        Lcd.fillRect(0,16,480,(320-24),LCD_BLACK);
        for(int i=0; i<28; i++) {
          Lcd.setTextColor(color, LCD_BLACK);
          Lcd.setCursor(0+(i*10),16+(i*10));
          Lcd.print_5x8("LCD test");
          color += 0x1234;
        }
        break;
    }

    Lcd.setTextColor(LCD_WHITE, LCD_BLACK);
    Lcd.setCursor((480-(6*2*12)), 0);
    sprintf(string_buf, "% 3.3f fps ", (1000000.0f / system_time_elapsed_us(get_system_time_us(), start_time)));
    Lcd.print_5x8(string_buf);

    count++;
    if(count > 4) count = 0;

    if(Gamepad.is_btn_pressed(BTN_SELECT) && Gamepad.is_btn_pressed(BTN_START)) {
      Lcd.setTextColor(LCD_WHITE, LCD_BLACK);
      return;
    }
  }
}

void menu_touch_test(void) {
  Lcd.setTextSize(1);
  Lcd.setCursor(0,320-8);
  Lcd.print_5x8("press SELECT & START to exit menu");
  Lcd.setTextSize(2);
  Lcd.setCursor(0,0);
  Lcd.print_5x8("Touch test");

  time_ms_t touch_timer;
  char string_buf[32];

  sprintf(string_buf, "x:    0, y:    0");
  Lcd.setCursor(0,16);
  Lcd.print_5x8(string_buf);

  touch_point_t point;
  touch_point_t prev_point = {.x = -1, .y = -1};

  while(1) {
    sleep_ms(10);

    if(Touchscreen.is_touched()) {
      point = Touchscreen.get_touch_point();
      if(point.y >= 32 && point.y < (320-8)) {
        sprintf(string_buf, "x: % 4d, y: % 4d", point.x, point.y);
        Lcd.setCursor(0,16);
        Lcd.print_5x8(string_buf);
        if(prev_point.x != -1 && prev_point.y != -1) {
          Lcd.drawLine(prev_point.x, prev_point.y, point.x, point.y, LCD_WHITE);
        } else {
          //Lcd.fillCircle(point.x, point.y, 1, LCD_WHITE);
          Lcd.drawPixel(point.x, point.y, LCD_WHITE);
        }
        prev_point = point;
      }
    } else {
      prev_point = {.x = -1, .y = -1};
    }

    if(Gamepad.is_btn_pressed(BTN_START)) {
      Lcd.fillRect(0,32,480,(320-40),LCD_BLACK);
    }

    if(Gamepad.is_btn_pressed(BTN_SELECT) && Gamepad.is_btn_pressed(BTN_START)) {
      return;
    }
  }
}

void menu_audio_test(void) {
  Lcd.setTextSize(1);
  Lcd.setCursor(0,320-8);
  Lcd.print_5x8("press SELECT & START to exit menu");
  Lcd.setTextSize(2);
  Lcd.setCursor(0,0);
  Lcd.print_5x8("Audio test");

  Lcd.setCursor(0,16);
  Lcd.print_5x8("press A to play sound");

  music_note_t test_notes[4] = {
    {0, 4, 0},   // C4
    {1, 4, 4},   // E4
    {2, 4, 7},   // G4
    {3, 5, 0}    // C5
  };

  music_table_t test_music = {
    .len = 4,
    .note_duration_ms = 100,
    .notes = test_notes
  };

  while(1) {
    sleep_ms(100);

    if(Gamepad.is_btn_pressed(BTN_A)) {
      Audio.play_music(&test_music);
    }

    if(Gamepad.is_btn_pressed(BTN_SELECT) && Gamepad.is_btn_pressed(BTN_START)) {
      return;
    }
  }
}

void menu_vibration_test(void) {
  Lcd.setTextSize(1);
  Lcd.setCursor(0,320-8);
  Lcd.print_5x8("press SELECT & START to exit menu");
  Lcd.setTextSize(2);
  Lcd.setCursor(0,0);
  Lcd.print_5x8("Vibration test");

  uint16_t freq_l = 200, freq_r = 200;
  uint8_t power_l = 20, power_r = 20;

  while(1) {
    sleep_ms(100);
    char string_buf[32];

    sprintf(string_buf, "LRA(L) : freq: % 5d, power: % 4d", freq_l, power_l);
    Lcd.setCursor(0,16);
    Lcd.print_5x8(string_buf);
    sprintf(string_buf, "LRA(R) : freq: % 5d, power: % 4d", freq_r, power_r);
    Lcd.setCursor(0,32);
    Lcd.print_5x8(string_buf);

    if(Gamepad.is_btn_pressed(BTN_S1_UP)) {
      if(freq_l < 4000) freq_l += 10;
    }
    if(Gamepad.is_btn_pressed(BTN_S1_DOWN)) {
      if(freq_l > 10) freq_l -= 10;
    }
    if(Gamepad.is_btn_pressed(BTN_S1_LEFT)) {
      if(power_l > 0) power_l -= 1;
    }
    if(Gamepad.is_btn_pressed(BTN_S1_RIGHT)) {
      if(power_l < 100) power_l += 1;
    }

    if(Gamepad.is_btn_pressed(BTN_S2_UP)) {
      if(freq_r < 4000) freq_r += 10;
    }
    if(Gamepad.is_btn_pressed(BTN_S2_DOWN)) {
      if(freq_r > 10) freq_r -= 10;
    }
    if(Gamepad.is_btn_pressed(BTN_S2_LEFT)) {
      if(power_r > 0) power_r -= 1;
    }
    if(Gamepad.is_btn_pressed(BTN_S2_RIGHT)) {
      if(power_r < 100) power_r += 1;
    }

    if(Gamepad.is_btn_pressed(BTN_SL)) {
      Vibration.set_vibration(VIBRATION_L, freq_l, power_l);
    } else {
      Vibration.set_vibration(VIBRATION_L, 0, 0);
    }

    if(Gamepad.is_btn_pressed(BTN_SR)) {
      Vibration.set_vibration(VIBRATION_R, freq_r, power_r);
    } else {
      Vibration.set_vibration(VIBRATION_R, 0, 0);
    }

    if(Gamepad.is_btn_pressed(BTN_SELECT) && Gamepad.is_btn_pressed(BTN_START)) {
      Vibration.set_vibration(VIBRATION_L, 0, 0);
      Vibration.set_vibration(VIBRATION_R, 0, 0);
      return;
    }
  }
}

void menu_bat_test(void) {
  Lcd.setTextSize(1);
  Lcd.setCursor(0,320-8);
  Lcd.print_5x8("press SELECT & START to exit menu");
  Lcd.setTextSize(2);
  Lcd.setCursor(0,0);
  Lcd.print_5x8("Battery & Power test");

  while(1) {
    sleep_ms(100);
    char string_buf[32];

    sprintf(string_buf, "Vin : %01.3fV", Power.get_input_voltage());
    Lcd.setCursor(0,16);
    Lcd.print_5x8(string_buf);

    sprintf(string_buf, "Battery : %s", Charger.get_battery_exist() ? "Yes" : "No");
    Lcd.setCursor(0,16*2);
    Lcd.print_5x8(string_buf);
    sprintf(string_buf, "Level : % 3.1f%% (%01.3fV)", Charger.get_bat_level(), Charger.get_bat_voltage());
    Lcd.setCursor(0,16*3);
    Lcd.print_5x8(string_buf);
    sprintf(string_buf, "Charging : %s | Fault : 0x%02X\n", Charger.get_charging_status() ? "Yes" : "No", Charger.get_fault_status());
    Lcd.setCursor(0,16*4);
    Lcd.print_5x8(string_buf);

    if(Gamepad.is_btn_pressed(BTN_SELECT) && Gamepad.is_btn_pressed(BTN_START)) {
      return;
    }
  }
}

void menu_temp_test(void) {
  Lcd.setTextSize(1);
  Lcd.setCursor(0,320-8);
  Lcd.print_5x8("press SELECT & START to exit menu");
  Lcd.setTextSize(2);
  Lcd.setCursor(0,0);
  Lcd.print_5x8("Temperature test");

  while(1) {
    sleep_ms(100);
    char string_buf[32];
    sprintf(string_buf, "TEMP_BUILTIN     : %.1fC", Temperature.get_temp(TEMP_BUILTIN));
    Lcd.setCursor(0,16);
    Lcd.print_5x8(string_buf);
    sprintf(string_buf, "TEMP_SOUTHBRIDGE : %.1fC", Temperature.get_temp(TEMP_SOUTHBRIDGE));
    Lcd.setCursor(0,16*2);
    Lcd.print_5x8(string_buf);
    sprintf(string_buf, "TEMP_NTC         : %.1fC", Temperature.get_temp(TEMP_NTC));
    Lcd.setCursor(0,16*3);
    Lcd.print_5x8(string_buf);

    if(Gamepad.is_btn_pressed(BTN_SELECT) && Gamepad.is_btn_pressed(BTN_START)) {
      return;
    }
  }
}

void menu_ir_test(void) {
  Lcd.setTextSize(1);
  Lcd.setCursor(0,320-8);
  Lcd.print_5x8("press SELECT & START to exit menu");
  Lcd.setTextSize(2);
  Lcd.setCursor(0,0);
  Lcd.print_5x8("IR comm test");

  Lcd.setCursor(0,16);
  Lcd.print_5x8("press START to change format");

  led_config_t led_config = {.mode = LED_OFF, .brightness = 255, .blink_interval_ms = 100};
  LedCtrl.set_config(LED_CONTROL_1, led_config); // RX indicator
  LedCtrl.set_config(LED_CONTROL_4, led_config); // TX indicator

  Ir.set_enable_tx(true, IR_FORMAT_MANUAL);
  Ir.set_enable_rx(true, IR_FORMAT_MANUAL);
  
  while(1) {
    sleep_ms(100);
    char string_buf[32];

    sprintf(string_buf, "format setting: %s", Ir.get_rx_format() == IR_FORMAT_NEC ? "NEC" : "RAW");
    Lcd.setCursor(0,16*2);
    Lcd.print_5x8(string_buf);
    if(Ir.is_data_ready() && Ir.get_rx_format() == Ir.get_rx_data_format()) {
      if(Ir.get_rx_data_format() == IR_FORMAT_MANUAL) {
        sprintf(string_buf, "data: %d pulses", Ir.get_raw_data_pulses());
        Lcd.setCursor(0,16*3);
        Lcd.print_5x8(string_buf);
        sprintf(string_buf, "%d, %d, %d, %d ... ", ((uint16_t*)Ir.rx_data_buf)[0], ((uint16_t*)Ir.rx_data_buf)[1], ((uint16_t*)Ir.rx_data_buf)[2], ((uint16_t*)Ir.rx_data_buf)[3]);
        Lcd.setCursor(0,16*4);
        Lcd.print_5x8(string_buf);
        // Lcd.setCursor(0,16*5);
        // Lcd.print_5x8("press A to send data");
      } else if (Ir.get_rx_data_format() == IR_FORMAT_NEC) {
        Lcd.setCursor(0,16*3);
        Lcd.print_5x8("data:");
        sprintf(string_buf, "%02X %02X %02X %02X", Ir.rx_data_buf[0], Ir.rx_data_buf[1], Ir.rx_data_buf[2], Ir.rx_data_buf[3]);
        Lcd.setCursor(0,16*4);
        Lcd.print_5x8(string_buf);
        Lcd.setCursor(0,16*5);
        Lcd.print_5x8("press A to send data");
      }
    }

    if(Gamepad.is_btn_pressed(BTN_START)) {
      uint8_t temp = (uint8_t)Ir.get_rx_format();
      temp++;
      if(temp > IR_FORMAT_NEC) temp = IR_FORMAT_MANUAL;
      Ir.set_enable_rx(true, (enum ir_format)temp);
      Lcd.fillRect(0,16*3,480,(320-56),LCD_BLACK);
    }

    if(Gamepad.is_btn_pressed(BTN_A)) {
      if(Ir.is_data_ready()) {
        LedCtrl.set_mode(LED_CONTROL_4, LED_BLINK_ONCE);
        Ir.send_bridge_tx_data(Ir.get_rx_data_format(), Ir.rx_data_buf, Ir.rx_data_len);
      }
      sleep_ms(500);
    } 

    if(Gamepad.is_btn_pressed(BTN_SELECT) && Gamepad.is_btn_pressed(BTN_START)) {
      Ir.set_enable_tx(false, IR_FORMAT_MANUAL);
      Ir.set_enable_rx(false, IR_FORMAT_MANUAL);
      return;
    }
  }
}

void menu_imu_test(void) {
  Lcd.setTextSize(1);
  Lcd.setCursor(0,320-8);
  Lcd.print_5x8("press SELECT & START to exit menu");
  Lcd.setTextSize(2);
  Lcd.setCursor(0,0);
  Lcd.print_5x8("IMU test");

  Imu.set_enable(true, true);

  while(1) {
    sleep_ms(10);
    char string_buf[32];
    //test
    sprintf(string_buf, "accel x: %2.4f ", Imu.get_accel_x());
    Lcd.setCursor(0,16);
    Lcd.print_5x8(string_buf);
    sprintf(string_buf, "accel y: %2.4f ", Imu.get_accel_y());
    Lcd.setCursor(0,32);
    Lcd.print_5x8(string_buf);
    sprintf(string_buf, "accel z: %2.4f ", Imu.get_accel_z());
    Lcd.setCursor(0,48);
    Lcd.print_5x8(string_buf);

    sprintf(string_buf, "gyro x: %4.2f ", Imu.get_gyro_x());
    Lcd.setCursor(0,64);
    Lcd.print_5x8(string_buf);
    sprintf(string_buf, "gyro y: %4.2f ", Imu.get_gyro_y());
    Lcd.setCursor(0,80);
    Lcd.print_5x8(string_buf);
    sprintf(string_buf, "gyro z: %4.2f ", Imu.get_gyro_z());
    Lcd.setCursor(0,96);
    Lcd.print_5x8(string_buf);

    if(Gamepad.is_btn_pressed(BTN_SELECT) && Gamepad.is_btn_pressed(BTN_START)) {
      Imu.set_enable(false, false);
      return;
    }
  }
}

void menu_sd_test(void) {
  Lcd.setTextSize(1);
  Lcd.setCursor(0,320-8);
  Lcd.print_5x8("press SELECT & START to exit menu");
  Lcd.setTextSize(2);
  Lcd.setCursor(0,0);
  Lcd.print_5x8("SD card test");

  Lcd.setCursor(0,16);
  Lcd.print_5x8("Loading...");
  char string_buf[32];

  sd_card_t *sd_card_p = sd_get_by_num(0);

  int ds = sd_card_p->init(sd_card_p);
  if (STA_NODISK & ds || STA_NOINIT & ds) {
      printf("SD card initialization failed\n");
  }

  size_t au_size_bytes;
  bool ok = sd_allocation_unit(sd_card_p, &au_size_bytes);

  while(1) {
    sleep_ms(100);

    sprintf(string_buf, "SD card : %s", (STA_NODISK & ds) ? "not inserted" : "inserted    ");
    Lcd.setCursor(0,16);
    Lcd.print_5x8(string_buf);
    if(ok) {
      sprintf(string_buf, "mount : %s", sd_card_p->state.mounted ? "not mounted" : "mounted    ");
      Lcd.setCursor(0,16*2);
      Lcd.print_5x8(string_buf);
      if(au_size_bytes > (1<<20)) {
        sprintf(string_buf, "size : %zu MB (%zu sectors)", au_size_bytes / (1<<20), au_size_bytes / sd_block_size);
      } else {
        sprintf(string_buf, "size : %zu bytes (%zu sectors)", au_size_bytes, au_size_bytes / sd_block_size);
      }
      Lcd.setCursor(0,16*3);
      Lcd.print_5x8(string_buf);
    }

    if(Gamepad.is_btn_pressed(BTN_SELECT) && Gamepad.is_btn_pressed(BTN_START)) {
      return;
    }
  }
}

void bridge_do_cmd(const bridge_msg_t* msg) {
  enum bridge_cmd command = (enum bridge_cmd)msg->cmd;
  last_bridge_cmd_time = get_system_time_ms();

  switch (command)
  {
  case CMD_TEMPERATURE_DATA:
    Temperature.recv_bridge_data(msg->payload, msg->payload_size);
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
  case CMD_GAMEPAD_RAW_DATA:
    Gamepad.recv_bridge_raw_data(msg->payload, msg->payload_size);
    break;
  case CMD_IR_RX_DATA:
    Ir.recv_bridge_rx_data(msg->payload, msg->payload_size);
    break;
  case CMD_IMU_ACCEL_DATA:
    Imu.recv_bridge_accel_data(msg->payload, msg->payload_size);
    break;
  case CMD_IMU_GYRO_DATA:
    Imu.recv_bridge_gyro_data(msg->payload, msg->payload_size);
    break;
  default:
    break;
  }
}
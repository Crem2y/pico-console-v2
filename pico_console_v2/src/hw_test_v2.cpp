#include <stdio.h>

// pico libraries
#include "pico/stdlib.h"
#include "pico/multicore.h"

// headers
#include "hw_test.hpp"

// drivers
#include "uart_log.h"
#include "uart_bridge.h"
#include "led_builtin.h"
#include "led_status.hpp"
#include "li_battery.hpp"
#include "ili9488_40_hstx.hpp"
#include "system_time.h"
#include "temp_builtin.h"
#include "xpt2046.hpp"

// middlewares
#include "gamepad.hpp"
// #include "sound_system.hpp"
#include "led_control.hpp"
#include "temperature.hpp"

// hw lib init
ledStatus Led = ledStatus(26,27,28,29);
ili9488_40 Lcd = ili9488_40(12,13,14,15, 16,17);
xpt2046 Touch = xpt2046(spi0, 19,20,18,21, -1);
liBattery Bat = liBattery(28, ((float)1/2));

// middleware lib init
ledControl LedCtrl = ledControl(&Led);
gamepad Gamepad = gamepad();
//soundSystem Sound = soundSystem();
temperature Temperature = temperature();

void core1_entry();
void bridge_do_cmd(bridge_protocol_t* cmd);

//////// function ////////

int main() { // uses core 0 to sub core
  uartLog_init(uart0, 0, 1, 115200);
  uart_bridge_init(uart1, 4, 5, 115200);
  set_bridge_do_cmd(bridge_do_cmd);
  LedCtrl.init();

  sleep_ms(100);
  LedCtrl.set_brightness(LED_CONTROL_BUILT_IN, 255);
  LedCtrl.set_blink_interval(LED_CONTROL_BUILT_IN, 500);
  LedCtrl.set_mode(LED_CONTROL_BUILT_IN, LED_BLINK_REPEAT);
  LedCtrl.update();

  multicore_launch_core1(core1_entry);

  // initalizing hardwares
  // LedCtrl.set_brightness(LED_CONTROL_1, 255);
  // LedCtrl.set_brightness(LED_CONTROL_2, 255);
  // LedCtrl.set_brightness(LED_CONTROL_3, 255);
  // LedCtrl.set_brightness(LED_CONTROL_4, 255);
  // LedCtrl.set_mode(LED_CONTROL_1, LED_ON);
  // LedCtrl.set_mode(LED_CONTROL_2, LED_ON);
  // LedCtrl.set_mode(LED_CONTROL_3, LED_ON);
  // LedCtrl.set_mode(LED_CONTROL_4, LED_ON);
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
  Touch.init();
  LOG_PRINTF("Touch ok\n");
  Lcd.setCursor(0,0);
  Lcd.print_5x8("Gamepad init...");
  Gamepad.init();
  LOG_PRINTF("Gamepad ok\n");
  // Lcd.setCursor(0,0);
  // Lcd.print_5x8("TEMP init...");
  // Temperature.init();
  // LOG_PRINTF("TEMP ok\n");
  Lcd.setCursor(0,0);
  Lcd.print_5x8("               ");
  LOG_PRINTF("all HWs ok!\n");
  LOG_PRINTF("core freq = %ld hz\n", SYS_CLK_KHZ * 1000);
  // hardware initalized

  // boot animation
  Lcd.setTextSize(2);
  for(int i=0; i<160; i+=5) {
    Lcd.setCursor(140,i-5);
    Lcd.print_5x8("               ");
    Lcd.setCursor(140,i);
    Lcd.print_5x8("PICO CONSOLE V2");
    sleep_ms(50);
  }

  Lcd.setCursor(372,304);
  Lcd.print_5x8("by Crem2y");
  Lcd.setTextSize(1);
  Lcd.setCursor(190,200);
  Lcd.print_5x8("press START");
  Lcd.setCursor(170,210);
  Lcd.print_5x8("or touch the screen");

//  menu_lcd_test();

  time_ms_t touch_time_ms = 0; //placeholder
  time_ms_t display_time_ms = 0;
  bool display_text = false;
  while(true) {
    bridge_handle();
    Gamepad.update();
    if(Gamepad.is_btn_pressed(BTN_START)) break;

    if(system_time_elapsed_ms(get_system_time_ms(), display_time_ms) > 1000) {
      display_time_ms = get_system_time_ms();
      if(display_text) {
        Lcd.fillRect(190,200,66,8,LCD_BLACK);
      } else {
        Lcd.setCursor(190,200);
        Lcd.print_5x8("press START");
      }
      display_text = !display_text;
    }

    // touch test
    if(system_time_elapsed_ms(get_system_time_ms(), touch_time_ms) > 100) {
      touch_time_ms = get_system_time_ms();
      Touch.get_touch_data();
      // LOG_PRINTF("x: %d, y: %d, z1: %d, z2: %d\n", Touch.touch_data.x, Touch.touch_data.y, Touch.touch_data.z1, Touch.touch_data.z2);
    }
    if(Touch.touch_data.z1 > 50) break;
  }

  LOG_PRINTF("go to main loop\n");
  multicore_fifo_push_blocking(1);

  // Gamepad.update();
  // if(!Gamepad.is_btn_pressed(BTN_SELECT)) {
  //   Sound.set_waning(32);
  //   Sound.play_music_ex(Music_Boot_ex, 10, 100);
  // }
  //Sound.set_mute(true);
  // boot sequence end
  
  // Gamepad.update();

  while (true) {
    // sleep_ms(10);
    bridge_handle();
    Gamepad.update();
    //Bat.get_level();
    LedCtrl.update();
    // Temperature.update();
  }

  return 0;
}

void core1_entry() { // uses core 1 to main core

  multicore_fifo_pop_blocking(); // wait until boot process is done

  // LedCtrl.set_mode(LED_CONTROL_1, LED_DARKER);
  // LedCtrl.set_mode(LED_CONTROL_2, LED_DARKER);
  // LedCtrl.set_mode(LED_CONTROL_3, LED_DARKER);
  // LedCtrl.set_mode(LED_CONTROL_4, LED_DARKER);

  bridge_protocol_t response_cmd;
  uint8_t temp_payload[PAYLOAD_MAX_SIZE];
  temp_payload[0] = 0x60;
  response_cmd = bridge_protocol_create(CMD_AUDIO_PCM_DATA, 1, temp_payload);
  bridge_cmd_queue_push(response_cmd);
  sleep_ms(100);
  temp_payload[0] = 0x70;
  response_cmd = bridge_protocol_create(CMD_AUDIO_PCM_DATA, 1, temp_payload);
  bridge_cmd_queue_push(response_cmd);

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
    Lcd.print_5x8("LED test");
    Lcd.setCursor(16,32);
    Lcd.print_5x8("LCD test");
    Lcd.setCursor(16,48);
    Lcd.print_5x8("DAC test");
    Lcd.setCursor(16,64);
    Lcd.print_5x8("Battery test");
    Lcd.setCursor(16,80);
    Lcd.print_5x8("Temperature test");
    Lcd.setCursor(16,96);
    Lcd.print_5x8("IR LED test");
    Lcd.setCursor(16,112);
    Lcd.print_5x8("SD card test");

    Lcd.setTextSize(1);
    Lcd.setCursor(0,224);
    Lcd.print_5x8("press up/down to move cursor");
    Lcd.setCursor(0,232);
    Lcd.print_5x8("press A or START to select");

    while(1) {
      sleep_ms(100);

      Lcd.setTextSize(1);
      sprintf(string_buf, "BAT:% 3.1f%%", Bat.level);
      Lcd.setCursor(254,0);
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

      if(Gamepad.is_btn_pressed(BTN_A) || Gamepad.is_btn_pressed(BTN_START)) {
        Lcd.fillScreen(0x0000);
        switch (cursor_x)
        {
        case MAIN_BTN_TEST:
          menu_btn_test();
          break;
        /*
        case MAIN_LED_TEST:
          menu_led_test();
          break;
        case MAIN_LCD_TEST:
          menu_lcd_test();
          break;
        case MAIN_DAC_TEST:
          menu_dac_test();
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
        case MAIN_SD_TEST:
          menu_sd_test();
          break;
        */
        }
        goto main_menu_loop;
      }
    }
  }
}

//////// test menus ////////

void menu_btn_test(void) {
  Lcd.setTextSize(1);
  Lcd.setCursor(0,232);
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
/*
void menu_led_test(void) {
  Lcd.setTextSize(1);
  Lcd.setCursor(0,232);
  Lcd.print_5x8("press SELECT & START to exit menu");
  Lcd.setTextSize(2);
  Lcd.setCursor(0,0);
  Lcd.print_5x8("LED test");

  LedCtrl.set_brightness(LED_CONTROL_1, 255);
  LedCtrl.set_breathing_step(LED_CONTROL_1, 5);
  LedCtrl.set_mode(LED_CONTROL_1, LED_BREATHING_REPEAT);
  LedCtrl.set_brightness(LED_CONTROL_2, 255);
  LedCtrl.set_breathing_step(LED_CONTROL_2, 5);
  LedCtrl.set_mode(LED_CONTROL_2, LED_BREATHING_REPEAT);
  LedCtrl.set_brightness(LED_CONTROL_3, 255);
  LedCtrl.set_breathing_step(LED_CONTROL_3, 5);
  LedCtrl.set_mode(LED_CONTROL_3, LED_BREATHING_REPEAT);
  LedCtrl.set_brightness(LED_CONTROL_4, 255);
  LedCtrl.set_breathing_step(LED_CONTROL_4, 5);
  LedCtrl.set_mode(LED_CONTROL_4, LED_BREATHING_REPEAT);

  while(1) {
    sleep_ms(100);

    if(Gamepad.is_btn_pressed(BTN_SELECT) && Gamepad.is_btn_pressed(BTN_START)) {
      LedCtrl.set_mode(LED_CONTROL_1, LED_OFF);
      LedCtrl.set_mode(LED_CONTROL_2, LED_OFF);
      LedCtrl.set_mode(LED_CONTROL_3, LED_OFF);
      LedCtrl.set_mode(LED_CONTROL_4, LED_OFF);
      return;
    }
  }
}
*/
void menu_lcd_test(void) {
  Lcd.setTextSize(1);
  Lcd.setCursor(0,312);
  Lcd.print_5x8("press SELECT & START to exit menu");
  Lcd.setTextSize(2);
  Lcd.setCursor(0,0);
  Lcd.print_5x8("LCD test");

  int count = 0;

  while(1) {
    sleep_ms(500);

    switch(count) {
      case 0:
        Lcd.fillRect(0,16,480,296,LCD_WHITE);
        break;
      case 1:
        Lcd.fillRect(0,16,480,296,LCD_RED);
        break;
      case 2:
        Lcd.fillRect(0,16,480,296,LCD_GREEN);
        break;
      case 3:
        Lcd.fillRect(0,16,480,296,LCD_BLUE);
        break;
      case 4:
        Lcd.fillRect(0,16,480,296,LCD_BLACK);
        break;
    }

    count++;
    if(count > 4) count = 0;

    // if(Gamepad.is_btn_pressed(BTN_SELECT) && Gamepad.is_btn_pressed(BTN_START)) {
    //   return;
    // }
  }
}
/*
void menu_dac_test(void) {
  Lcd.setTextSize(1);
  Lcd.setCursor(0,232);
  Lcd.print_5x8("press SELECT & START to exit menu");
  Lcd.setTextSize(2);
  Lcd.setCursor(0,0);
  Lcd.print_5x8("DAC test");

  Lcd.setCursor(0,16);
  Lcd.print_5x8("press A to play sound");

  while(1) {
    sleep_ms(100);
    
    if(Gamepad.is_btn_pressed(BTN_A)) {
      Sound.play_music_ex(Music_Test, 10, 100);
      Sound.set_mute(true);
    }

    if(Gamepad.is_btn_pressed(BTN_SELECT) && Gamepad.is_btn_pressed(BTN_START)) {
      return;
    }
  }
}

void menu_bat_test(void) {
  Lcd.setTextSize(1);
  Lcd.setCursor(0,232);
  Lcd.print_5x8("press SELECT & START to exit menu");
  Lcd.setTextSize(2);
  Lcd.setCursor(0,0);
  Lcd.print_5x8("Battery test");

  while(1) {
    sleep_ms(100);
    char string_buf[32];
    sprintf(string_buf, "bat level   : % 3.1f%%", Bat.level);
    Lcd.setCursor(0,16);
    Lcd.print_5x8(string_buf);
    sprintf(string_buf, "bat voltage : %01.3fV", Bat.voltage);
    Lcd.setCursor(0,32);
    Lcd.print_5x8(string_buf);

    if(Gamepad.is_btn_pressed(BTN_SELECT) && Gamepad.is_btn_pressed(BTN_START)) {
      return;
    }
  }
}

void menu_temp_test(void) {
  Lcd.setTextSize(1);
  Lcd.setCursor(0,232);
  Lcd.print_5x8("press SELECT & START to exit menu");
  Lcd.setTextSize(2);
  Lcd.setCursor(0,0);
  Lcd.print_5x8("Temperature test");

  while(1) {
    sleep_ms(100);
    char string_buf[32];
    sprintf(string_buf, "TEMP_BUILTIN : % 3.1fC", (float)Temperature.get_temp(TEMP_BUILTIN) / 100.0f);
    Lcd.setCursor(0,16);
    Lcd.print_5x8(string_buf);

    if(Gamepad.is_btn_pressed(BTN_SELECT) && Gamepad.is_btn_pressed(BTN_START)) {
      return;
    }
  }
}

void menu_ir_test(void) {
  Lcd.setTextSize(1);
  Lcd.setCursor(0,232);
  Lcd.print_5x8("press SELECT & START to exit menu");
  Lcd.setTextSize(2);
  Lcd.setCursor(0,0);
  Lcd.print_5x8("IR LED test");

  Lcd.setCursor(0,16);
  Lcd.print_5x8("press A to turn on IR LED");
  
  while(1) {
    sleep_ms(10);

    if(Gamepad.is_btn_pressed(BTN_START)) {
      Ir.send_nec_format(true, 0x00, 0xff, 0x09); // 전원
      sleep_ms(500);
    } else if(Gamepad.is_btn_pressed(BTN_SELECT)) {
      Ir.send_nec_format(true, 0x00, 0xff, 0x40); // 외부입력
      sleep_ms(500);
    } else if(Gamepad.is_btn_pressed(BTN_A)) {
      Ir.send_nec_format(true, 0x00, 0xff, 0x16); // 선택
      sleep_ms(500);
    } else if(Gamepad.is_btn_pressed(BTN_B)) {
      Ir.send_nec_format(true, 0x00, 0xff, 0x10); // 나가기
      sleep_ms(500);
    } else if(Gamepad.is_btn_pressed(BTN_S1_UP)) {
      Ir.send_nec_format(true, 0x00, 0xff, 0x11); // 상
      sleep_ms(500);
    } else if(Gamepad.is_btn_pressed(BTN_S1_DOWN)) {
      Ir.send_nec_format(true, 0x00, 0xff, 0x04); // 하
      sleep_ms(500);
    } else if(Gamepad.is_btn_pressed(BTN_S1_LEFT)) {
      Ir.send_nec_format(true, 0x00, 0xff, 0x12); // 좌
      sleep_ms(500);
    } else if(Gamepad.is_btn_pressed(BTN_S1_RIGHT)) {
      Ir.send_nec_format(true, 0x00, 0xff, 0x4d); // 우
      sleep_ms(500);
    } else {
      Ir.manual_off();
    } 

    if(Gamepad.is_btn_pressed(BTN_SELECT) && Gamepad.is_btn_pressed(BTN_START)) {
      return;
    }
  }
}

void menu_sd_test(void) {
  Lcd.setTextSize(1);
  Lcd.setCursor(0,232);
  Lcd.print_5x8("press SELECT & START to exit menu");
  Lcd.setTextSize(2);
  Lcd.setCursor(0,0);
  Lcd.print_5x8("SD card test");

  sleep_ms(100);
  char string_buf[32];
  sprintf(string_buf, "SD card : %s", Sdcard.card_check() ? "inserted    " : "not inserted");
  Lcd.setCursor(0,16);
  Lcd.print_5x8(string_buf);

  int ret = Sdcard.card_init();
  if(ret < 0) {
    Sdcard.card_deinit();
  }

  sprintf(string_buf, "inited : %s", Sdcard.info.is_inited ? "yes" : "error");
  Lcd.setCursor(0,32);
  Lcd.print_5x8(string_buf);

  if(!Sdcard.info.is_inited) {
    sprintf(string_buf, "(%d)", ret);
    Lcd.print_5x8(string_buf);

  } else {
    Lcd.setCursor(0,48);
    Lcd.print_5x8("SD card type : ");
  
    switch (Sdcard.info.type)
    {
    case SD_TYPE_SDSC:
      Lcd.print_5x8("SDSC");
      break;
    case SD_TYPE_SDHC:
      if(Sdcard.info.size > 34359738368) { // 34359738368 = 1024^3
        Lcd.print_5x8("SDXC");
      } else {
        Lcd.print_5x8("SDHC");
      }
      break;
    default:
      Lcd.print_5x8("UNKNOWN");
      break;
    }

    Lcd.setCursor(0,64);
    Lcd.print_5x8("size : ");
    sprintf(string_buf, "%llu MB", (Sdcard.info.size / 1000000));
    Lcd.print_5x8(string_buf);

    int ret;
    uint8_t buf[512] = {0,};
    ret = Sdcard.sector_read(0, buf);
    if(ret < 0) {
      LOG_PRINTF("sector_read error(%d)\n", ret);
    }
    LOG_PRINTF("SD SECTOR[0] : ");
    for(int i=0; i<512; i++)
    {
      LOG_PRINTF("%02X ", buf[i]);
    }
    LOG_PRINTF("\n");

    ret = Sdcard.sector_read(1, buf);
    if(ret < 0) {
      LOG_PRINTF("sector_read error(%d)\n", ret);
    }
    LOG_PRINTF("SD SECTOR[1] : ");
    for(int i=0; i<512; i++)
    {
      LOG_PRINTF("%02X ", buf[i]);
    }
    LOG_PRINTF("\n");
  }

  while(1) {
    sleep_ms(100);

    if(Gamepad.is_btn_pressed(BTN_SELECT) && Gamepad.is_btn_pressed(BTN_START)) {
      return;
    }
  }
}
*/

void bridge_do_cmd(bridge_protocol_t* cmd) {
  enum bridge_cmd command = cmd->cmd;
  switch (command)
  {
  case CMD_GAMEPAD_DATA:
  {
    //placeholder for gamepad data handling
    if(cmd->payload[0] == 0x00) {
      Gamepad.force_update(BTN_START, false);
    } else {
      Gamepad.force_update(BTN_START, true);
    }
    break;
  }
  default:
    break;
  }
}
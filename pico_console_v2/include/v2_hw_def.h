#pragma once

// pin definitions
#define PIN_LOG_TX      0
#define PIN_LOG_RX      1
//    #define HW_LOG_CH       uart0
    #define HW_LOG_BAUD     115200
#define PIN_SDIO_CLK    2
#define PIN_SDIO_CMD    3
#define PIN_SDIO_D0     4
#define PIN_SDIO_D1     5
#define PIN_SDIO_D2     6
#define PIN_SDIO_D3     7
#define PIN_PSRAM_CS    8
#define PIN_SDIO_DET    9
#define PIN_BUILTIN_LED 10
// reserved 11
#define PIN_BRIDGE_TX   12
#define PIN_BRIDGE_RX   13
    #define HW_BRIDGE_CH    uart0
    #define HW_BRIDGE_BAUD  921600
#define PIN_DP_CS       14
#define PIN_DP_DC       15
#define PIN_DP_SCK      16
#define PIN_DP_MOSI     17
#define PIN_DP_RST      18
#define PIN_DP_BL       19
#define PIN_TOUCH_MISO  20
#define PIN_TOUCH_CS    21
#define PIN_TOUCH_SCK   22
#define PIN_TOUCH_MOSI  23
    #define HW_TOUCH_CH     spi0
#define PIN_TOUCH_IRQ   24
#define PIN_SB_RESET    25
#define PIN_LED_WL_1    26
#define PIN_LED_WL_2    27
#define PIN_LED_WL_3    28
#define PIN_LED_WL_4    29
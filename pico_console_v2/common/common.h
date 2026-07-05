#pragma once

#define ENABLE_LED 1
#define ENABLE_PSRAM 1

#define ENABLE_RFBRIDGE 1
#if ENABLE_RFBRIDGE
  #ifdef ENABLE_LED
    #undef ENABLE_LED
  #endif
  #define ENABLE_LED 0
#endif
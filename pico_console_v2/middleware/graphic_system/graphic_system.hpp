#include "ili9488_40_hstx.hpp"
#include "psram_apsxx04.h"

#define GRAPHIC_BLOCK_SIZE 16
#define GRAPHIC_BLOCK_COUNT_H (ILI9488_TFTWIDTH / GRAPHIC_BLOCK_SIZE)
#define GRAPHIC_BLOCK_COUNT_V (ILI9488_TFTHEIGHT / GRAPHIC_BLOCK_SIZE)

class graphicSystem {
  public:
    graphicSystem(ili9488_40* display);

    void init(void);

    void update_full(void);
    void update_dirty(void);

  private:
    ili9488_40* display;
};
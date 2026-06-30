#include "ili9488_40_hstx.hpp"
#include "psram_apsxx04.h"

#define GRAPHIC_BLOCK_SIZE 16
#define GRAPHIC_BLOCK_COUNT_H (ILI9488_TFTWIDTH / GRAPHIC_BLOCK_SIZE)
#define GRAPHIC_BLOCK_COUNT_V (ILI9488_TFTHEIGHT / GRAPHIC_BLOCK_SIZE)

enum GRAPHIC_COLOR16 {
  GC_BLACK   = 0x0000,
  GC_BLUE    = 0x001F,
  GC_RED     = 0xF800,
  GC_GREEN   = 0x07E0,
  GC_CYAN    = 0x07FF,
  GC_MAGENTA = 0xF81F,
  GC_YELLOW  = 0xFFE0,
  GC_WHITE   = 0xFFFF
};

typedef uint16_t g_color_t;

typedef struct _g_pos_t {
  int16_t x;
  int16_t y;
} g_pos_t;

typedef struct _g_area_t {
  int16_t w;
  int16_t h;
} g_area_t;

class graphicSystem {
  public:
    graphicSystem(ili9488_40* display);

    void init(void);

    void update_full(void);
    void update_dirty(void);

    void draw_pixel(g_pos_t pos, g_color_t color) {}
    void draw_line(g_pos_t pos0, g_pos_t pos1, g_color_t color) {}

    void draw_rect(g_pos_t pos, g_area_t fill_area, g_color_t color) {}
    void fill_rect(g_pos_t pos, g_area_t fill_area, g_color_t color) {}
    void fill_screen(g_color_t color) {}

    void draw_triangle(g_pos_t pos0, g_pos_t pos1, g_pos_t pos2, g_color_t color) {}
    void fill_triangle(g_pos_t pos0, g_pos_t pos1, g_pos_t pos2, g_color_t color) {}

    void draw_circle(g_pos_t pos, int16_t r, g_color_t color) {}
    void fill_circle(g_pos_t pos, int16_t r, g_color_t color) {}

    void set_text_color(g_color_t c) {}
    void set_text_color(g_color_t c, g_color_t bg) {}

    void set_cursor(g_pos_t pos) {}
    void set_text_size(uint8_t s) {}

    void print_5x8(const char *s) {}

  private:
    ili9488_40* display;
};
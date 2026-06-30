#pragma once

#include <stdint.h>
#include "ili9488_40_hstx.hpp"
#include "psram_apsxx04.h"

#define ENABLE_ADAFRUIT_GFX_COMPATIBLE 1 // Enables Adafruit GFX compatible functions.
#define USE_TEXT_BUFFER 1 // Use a buffer for text output. Print speed is improved by approximately 20%, but transparent backgrounds cannot be used in this mode.

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

enum g_font {
  G_FONT_5X8,
  G_FONT_16,
};

class graphicSystem {
  public:
    graphicSystem(ili9488_40* display);

    void init(void);

    void update_full(void);
    void update_dirty(void);

    void draw_pixel(g_pos_t pos, g_color_t color);
    void draw_line(g_pos_t pos0, g_pos_t pos1, g_color_t color);

    void draw_rect(g_pos_t pos, g_area_t fill_area, g_color_t color);
    void fill_rect(g_pos_t pos, g_area_t fill_area, g_color_t color);
    void fill_screen(g_color_t color);

    void draw_picture(g_pos_t pos, g_area_t fill_area, const g_color_t *bitmap);

    void draw_triangle(g_pos_t pos0, g_pos_t pos1, g_pos_t pos2, g_color_t color);
    void fill_triangle(g_pos_t pos0, g_pos_t pos1, g_pos_t pos2, g_color_t color);

    void draw_circle(g_pos_t pos, int16_t r, g_color_t color);
    void fill_circle(g_pos_t pos, int16_t r, g_color_t color);

    void set_text_color(g_color_t c);
    void set_text_color(g_color_t c, g_color_t bg);

    void set_cursor(g_pos_t pos);

    void set_text_size(uint8_t s);
    void set_font(enum g_font font);
    void print(const char *s);

    void print_5x8(const char *s);
    size_t write_5x8(const uint8_t c);
    void draw_char_5x8(g_pos_t pos, unsigned char c, g_color_t color, g_color_t bg, uint8_t size);

//// display configuration
    void set_rotation(uint8_t r);
    void set_bright(uint32_t bright) { _display->set_bright(bright); }
    uint32_t get_bright(void) { return _display->get_bright(); }

#if ENABLE_ADAFRUIT_GFX_COMPATIBLE
//// Adafruit GFX compatible layer ////
    void begin(void) {
      init();
    }
    void drawPixel(int16_t x, int16_t y, uint16_t color) {
      draw_pixel((g_pos_t){x,y}, color);
    }
    void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) {
      draw_line((g_pos_t){x0,y0}, (g_pos_t){x1,y1}, color);
    }
    void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
      draw_rect((g_pos_t){x,y}, (g_area_t){w,h}, color);
    }
    void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
      fill_rect((g_pos_t){x,y}, (g_area_t){w,h}, color);
    }
    void fillScreen(uint16_t color) {
      fill_screen(color);
    }
    void drawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color) {
      draw_triangle((g_pos_t){x0,y0}, (g_pos_t){x1,y1}, (g_pos_t){x2,y2}, color);
    }
    void fillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color) {
      fill_triangle((g_pos_t){x0,y0}, (g_pos_t){x1,y1}, (g_pos_t){x2,y2}, color);
    }
    void drawCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color) {
      draw_circle((g_pos_t){x0,y0}, r, color);
    }
    void fillCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color) {
      fill_circle((g_pos_t){x0,y0}, r, color);
    }
    void setCursor(int16_t x, int16_t y) {
      set_cursor((g_pos_t){x,y});
    }
    void setTextColor(uint16_t c) {
      set_text_color(c);
    }
    void setTextColor(uint16_t c, uint16_t bg) {
      set_text_color(c, bg);
    }
    void setTextSize(uint8_t s) {
      set_text_size(s);
    }
    void setRotation(uint8_t r) {
      set_rotation(r);
    }
#endif

  private:
    ili9488_40* _display;

    int16_t _width, _height;
    g_pos_t cursor;

    g_color_t textcolor, textbgcolor;
    uint8_t textsize;
    enum g_font text_font;
    bool wrap;

    uint8_t rotation;
};
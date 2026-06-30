#include "graphic_system.hpp"
#include "glcdfont.h"

static g_color_t* frame_buffer = (g_color_t*)PSRAM_BASE;
static const size_t frame_buffer_size = (ILI9488_TFTWIDTH * ILI9488_TFTHEIGHT * sizeof(g_color_t));

#if USE_TEXT_BUFFER
static g_color_t text_draw_buffer[512];
#endif

graphicSystem::graphicSystem(ili9488_40* display) {
  _display = display;
}

void graphicSystem::init(void) {
  _display->begin();
  _width = ILI9488_TFTHEIGHT;
  _height = ILI9488_TFTWIDTH;

  cursor = {.x = 0, .y = 0};
  textcolor = GC_WHITE;
  textbgcolor = GC_BLACK;
  textsize = 8;
  text_font = G_FONT_5X8;
}

void graphicSystem::update_full(void) {
  g_pos_t pos = {0, 0};
  g_area_t fill_area = {_width, _height};
  draw_picture(pos, fill_area, frame_buffer);
}

void graphicSystem::update_dirty(void) {

}

void graphicSystem::draw_pixel(g_pos_t pos, g_color_t color) {
  _display->drawPixel(pos.x, pos.y, color);
}

void graphicSystem::draw_line(g_pos_t pos0, g_pos_t pos1, g_color_t color) {
  _display->drawLine(pos0.x, pos0.y, pos1.x, pos1.y, color);
}

void graphicSystem::draw_rect(g_pos_t pos, g_area_t fill_area, g_color_t color) {
  _display->drawRect(pos.x, pos.y, fill_area.w, fill_area.h, color);
}

void graphicSystem::fill_rect(g_pos_t pos, g_area_t fill_area, g_color_t color) {
  _display->fillRect(pos.x, pos.y, fill_area.w, fill_area.h, color);
}

void graphicSystem::fill_screen(g_color_t color) {
  _display->fillScreen(color);
}

void graphicSystem::draw_picture(g_pos_t pos, g_area_t fill_area, const g_color_t *picture) {
  _display->drawPicture(pos.x, pos.y, picture, fill_area.w, fill_area.h);
}

void graphicSystem::draw_triangle(g_pos_t pos0, g_pos_t pos1, g_pos_t pos2, g_color_t color) {

}

void graphicSystem::fill_triangle(g_pos_t pos0, g_pos_t pos1, g_pos_t pos2, g_color_t color) {
  
}

void graphicSystem::draw_circle(g_pos_t pos, int16_t r, g_color_t color) {
  int16_t f = 1 - r;
  int16_t ddF_x = 1;
  int16_t ddF_y = -2 * r;
  int16_t x = 0;
  int16_t y = r;

  drawPixel(pos.x  , pos.y+r, color);
  drawPixel(pos.x  , pos.y-r, color);
  drawPixel(pos.x+r, pos.y  , color);
  drawPixel(pos.x-r, pos.y  , color);

  while (x<y) {
    if (f >= 0) {
      y--;
      ddF_y += 2;
      f += ddF_y;
    }
    x++;
    ddF_x += 2;
    f += ddF_x;
  
    drawPixel(pos.x + x, pos.y + y, color);
    drawPixel(pos.x - x, pos.y + y, color);
    drawPixel(pos.x + x, pos.y - y, color);
    drawPixel(pos.x - x, pos.y - y, color);
    drawPixel(pos.x + y, pos.y + x, color);
    drawPixel(pos.x - y, pos.y + x, color);
    drawPixel(pos.x + y, pos.y - x, color);
    drawPixel(pos.x - y, pos.y - x, color);
  }
}

void graphicSystem::fill_circle(g_pos_t pos, int16_t r, g_color_t color) {

}

void graphicSystem::set_text_color(g_color_t c) {
  textcolor = c;
}

void graphicSystem::set_text_color(g_color_t c, g_color_t bg) {
  textcolor = c;
  textbgcolor = bg;
}

void graphicSystem::set_cursor(g_pos_t pos) {
  cursor = pos;
}

void graphicSystem::set_text_size(uint8_t s) {
  textsize = s;
}

void graphicSystem::set_font(enum g_font font) {
  text_font = font;
}

void graphicSystem::print(const char *s) {
  switch(text_font) {
  case G_FONT_5X8:
    print_5x8(s);
    break;
  case G_FONT_16:
    //print_16(s);
    break;
  default:
    print_5x8(s);
    break;
  }
}

void graphicSystem::print_5x8(const char *s) {
  while(*s) {
    write_5x8(*s);
    s++;
  }
}

size_t graphicSystem::write_5x8(const uint8_t c) {
    if (c == '\n') {
    cursor.y += textsize*8;
    cursor.x  = 0;
  } else if (c == '\r') {
    // skip em
  } else {
    draw_char_5x8(cursor, c, textcolor, textbgcolor, textsize);
    //_display->drawChar_5x8(cursor.x, cursor.y, c, textcolor, textbgcolor, textsize);
    cursor.x += textsize*6;
    if (wrap && (cursor.x > (_width - textsize*6))) {
      cursor.y += textsize*8;
      cursor.x = 0;
    }
  }
  return 1;
}

void graphicSystem::draw_char_5x8(g_pos_t pos, unsigned char c, g_color_t color, g_color_t bg, uint8_t size) {
  if((pos.x >= _width)            || // Clip right
     (pos.y >= _height)           || // Clip bottom
     ((pos.x + 6 * size - 1) < 0) || // Clip left
     ((pos.y + 8 * size - 1) < 0))   // Clip top
    return;

#if USE_TEXT_BUFFER
  for (int8_t i=0; i<6; i++ ) {
    uint8_t line;
    if (i == 5) 
      line = 0x0;
    else 
      line = font[(c*5)+i];

    for (int8_t j=0; j<8; j++) {
      if (line & 0x1) {
        if (size == 1) { // minimum size
          text_draw_buffer[i + j*6] = color;
        } else { // big size
          for (uint8_t px=0; px<size; px++) {
            for (uint8_t py=0; py<size; py++) {
              text_draw_buffer[((i*size)+px) + ((j*size)+py)*(6*size)] = color;
            }
          }
        } 
      } else {
        if (size == 1) { // minimum size
          text_draw_buffer[i + j*6] = bg;
        } else { // big size
          for (uint8_t px=0; px<size; px++) {
            for (uint8_t py=0; py<size; py++) {
              text_draw_buffer[((i*size)+px) + ((j*size)+py)*(6*size)] = bg;
            }
          }
        }
      }
      line >>= 1;
    }
  }
  g_area_t fill_area = {size*6, size*8};
  draw_picture(pos, fill_area, text_draw_buffer);
  //_display->drawPicture(pos.x, pos.y, text_draw_buffer, size*6, size*8);
#else
  for (int8_t i=0; i<6; i++ ) {
    uint8_t line;
    if (i == 5) 
      line = 0x0;
    else 
      line = font[(c*5)+i];
    for (int8_t j = 0; j<8; j++) {
      if (line & 0x1) {
        if (size == 1) // default size
          drawPixel(pos.x+i, pos.y+j, color);
        else {  // big size
          fillRect(pos.x+(i*size), pos.y+(j*size), size, size, color);
        } 
      } else if (bg != color) { // if bg == color, transparent background
        if (size == 1) // default size
          drawPixel(pos.x+i, pos.y+j, bg);
        else {  // big size
          fillRect(pos.x+(i*size), pos.y+(j*size), size, size, bg);
        }
      }
      line >>= 1;
    }
  }
#endif
}
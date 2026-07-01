/*
This is the core graphics library for all our displays, providing a common
set of graphics primitives (points, lines, circles, etc.).  It needs to be
paired with a hardware-specific library for each display device we carry
(to handle the lower-level functions).

Adafruit invests time and resources providing this open source code, please
support Adafruit & open-source hardware by purchasing products from Adafruit!
 
Copyright (c) 2013 Adafruit Industries.  All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

- Redistributions of source code must retain the above copyright notice,
  this list of conditions and the following disclaimer.
- Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.
*/

// Modified for SPFD5408 by Joao Lopes to work with SPFD5408

// Most of the drawing code in this library is from Adafruit GFX.

#include "graphic_system.hpp"
#include "glcdfont.h"

#if USE_FRAME_BUFFER
static g_color_t* frame_buffer = (g_color_t*)PSRAM_BASE;
static const size_t frame_buffer_size = (ILI9488_TFTWIDTH * ILI9488_TFTHEIGHT * sizeof(g_color_t));
#endif

#if USE_TEXT_BUFFER
static g_color_t text_draw_buffer[512];
//static g_color_t* text_draw_buffer = (g_color_t*)PSRAM_BASE;
#endif

#define swap(a, b) { int16_t t = a; a = b; b = t; }

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
  textsize = 1;
  text_font = G_FONT_5X8;
  wrap      = true;

#if USE_FRAME_BUFFER
  for(uint32_t i=0; i<frame_buffer_size/2; i++) {
    frame_buffer[i] = GC_BLACK;
  }
  update_full();
#endif
}

void graphicSystem::update_full(void) {
#if USE_FRAME_BUFFER
  _display->draw_picture(0, 0, frame_buffer, _width, _height);
#endif
}

void graphicSystem::update_dirty(void) {
#if USE_FRAME_BUFFER
  _display->draw_picture(0, 0, frame_buffer, _width, _height);
#endif
}

void graphicSystem::draw_pixel(int16_t x, int16_t y, g_color_t color) {
#if USE_FRAME_BUFFER
  frame_buffer[x + y * _width] = color;
#else
  _display->drawPixel(x, y, color);
#endif
}

void graphicSystem::draw_pixel(g_pos_t pos, g_color_t color) {
#if USE_FRAME_BUFFER
  frame_buffer[pos.x + pos.y * _width] = color;
#else
  _display->drawPixel(pos.x, pos.y, color);
#endif
}

void graphicSystem::draw_line(int16_t x0, int16_t y0, int16_t x1, int16_t y1, g_color_t color) {
  int16_t steep = abs(y1 - y0) > abs(x1 - x0);
  if (steep) {
    swap(x0, y0);
    swap(x1, y1);
  }

  if (x0 > x1) {
    swap(x0, x1);
    swap(y0, y1);
  }

  int16_t dx, dy;
  dx = x1 - x0;
  dy = abs(y1 - y0);

  int16_t err = dx / 2;
  int16_t ystep;

  if (y0 < y1) {
    ystep = 1;
  } else {
    ystep = -1;
  }

  for (; x0<=x1; x0++) {
    if (steep) {
      draw_pixel(y0, x0, color);
    } else {
      draw_pixel(x0, y0, color);
    }
    err -= dy;
    if (err < 0) {
      y0 += ystep;
      err += dx;
    }
  }
}

void graphicSystem::draw_line(g_pos_t pos0, g_pos_t pos1, g_color_t color) {
  int16_t x0 = pos0.x;
  int16_t y0 = pos0.y;
  int16_t x1 = pos1.x;
  int16_t y1 = pos1.y;

  int16_t steep = abs(y1 - y0) > abs(x1 - x0);
  if (steep) {
    swap(x0, y0);
    swap(x1, y1);
  }

  if (x0 > x1) {
    swap(x0, x1);
    swap(y0, y1);
  }

  int16_t dx, dy;
  dx = x1 - x0;
  dy = abs(y1 - y0);

  int16_t err = dx / 2;
  int16_t ystep;

  if (y0 < y1) {
    ystep = 1;
  } else {
    ystep = -1;
  }

  for (; x0<=x1; x0++) {
    if (steep) {
      draw_pixel(y0, x0, color);
    } else {
      draw_pixel(x0, y0, color);
    }
    err -= dy;
    if (err < 0) {
      y0 += ystep;
      err += dx;
    }
  }
}

void graphicSystem::draw_rect(int16_t x, int16_t y, int16_t w, int16_t h, g_color_t color) {
  _display->draw_fast_line_h(x, y, w, color);
  _display->draw_fast_line_h(x, y+h-1, w, color);
  _display->draw_fast_line_v(x, y, h, color);
  _display->draw_fast_line_v(x+w-1, y, h, color);
}

void graphicSystem::draw_rect(g_rect_t rect, g_color_t color) {
#if USE_FRAME_BUFFER
  for(int16_t y=0; y<rect.h; y++) {
    for(int16_t x=0; x<rect.w; x++) {
      if(y==0 || y==(rect.h-1) || x==0 || x==(rect.w-1)) {
        frame_buffer[(rect.x + x) + (rect.y + y) * _width] = color;
      }
    }
  }
#else
  int16_t x = rect.x;
  int16_t y = rect.y;
  int16_t w = rect.w;
  int16_t h = rect.h;

  _display->draw_fast_line_h(x, y, w, color);
  _display->draw_fast_line_h(x, y+h-1, w, color);
  _display->draw_fast_line_v(x, y, h, color);
  _display->draw_fast_line_v(x+w-1, y, h, color);
#endif
}

void graphicSystem::fill_rect(int16_t x, int16_t y, int16_t w, int16_t h, g_color_t color) {
#if USE_FRAME_BUFFER
  for(int16_t j=0; j<h; j++) {
    for(int16_t i=0; i<w; i++) {
      frame_buffer[(x + i) + (y + j) * _width] = color;
    }
  }
#else
  _display->fill_rect(x, y, w, h, color);
#endif
}

void graphicSystem::fill_rect(g_rect_t rect, g_color_t color) {
#if USE_FRAME_BUFFER
  for(int16_t y=0; y<rect.h; y++) {
    for(int16_t x=0; x<rect.w; x++) {
      frame_buffer[(rect.x + x) + (rect.y + y) * _width] = color;
    }
  }
#else
  _display->fill_rect(rect.x, rect.y, rect.w, rect.h, color);
#endif
}

void graphicSystem::fill_screen(g_color_t color) {
#if USE_FRAME_BUFFER
  for(int16_t y=0; y<_height; y++) {
    for(int16_t x=0; x<_width; x++) {
      frame_buffer[x + y * _width] = color;
    }
  }
#else
  _display->fill_screen(color);
#endif
}

void graphicSystem::draw_picture(int16_t x, int16_t y, int16_t w, int16_t h, const g_color_t *picture) {
#if USE_FRAME_BUFFER
  for(j=0; j<h; j++) {
    for(i=0; i<w; i++) {
      frame_buffer[(x + i) + (y + j) * w] = picture[x + y * w];
    }
  }
#else
  _display->draw_picture(x, y, w, h, picture);
#endif
}

void graphicSystem::draw_picture(g_rect_t rect, const g_color_t *picture) {
#if USE_FRAME_BUFFER
  for(int16_t y=0; y<rect.h; y++) {
    for(int16_t x=0; x<rect.w; x++) {
      frame_buffer[(rect.x + x) + (rect.y + y) * rect.w] = picture[x + y * rect.w];
    }
  }
#else
  _display->draw_picture(rect.x, rect.y, rect.w, rect.h, picture);
#endif
}

void graphicSystem::draw_triangle(g_pos_t pos0, g_pos_t pos1, g_pos_t pos2, g_color_t color) {

}

void graphicSystem::fill_triangle(g_pos_t pos0, g_pos_t pos1, g_pos_t pos2, g_color_t color) {
  
}

void graphicSystem::draw_circle(int16_t x0, int16_t y0, int16_t r, g_color_t color) {
  int16_t f = 1 - r;
  int16_t ddF_x = 1;
  int16_t ddF_y = -2 * r;
  int16_t x = 0;
  int16_t y = r;

  draw_pixel(x0  , y0+r, color);
  draw_pixel(x0  , y0-r, color);
  draw_pixel(x0+r, y0  , color);
  draw_pixel(x0-r, y0  , color);

  while (x<y) {
    if (f >= 0) {
      y--;
      ddF_y += 2;
      f += ddF_y;
    }
    x++;
    ddF_x += 2;
    f += ddF_x;
  
    draw_pixel(x0 + x, y0 + y, color);
    draw_pixel(x0 - x, y0 + y, color);
    draw_pixel(x0 + x, y0 - y, color);
    draw_pixel(x0 - x, y0 - y, color);
    draw_pixel(x0 + y, y0 + x, color);
    draw_pixel(x0 - y, y0 + x, color);
    draw_pixel(x0 + y, y0 - x, color);
    draw_pixel(x0 - y, y0 - x, color);
  }
}

void graphicSystem::draw_circle(g_pos_t pos, int16_t r, g_color_t color) {
  int16_t f = 1 - r;
  int16_t ddF_x = 1;
  int16_t ddF_y = -2 * r;
  int16_t x = 0;
  int16_t y = r;

  draw_pixel(pos.x  , pos.y+r, color);
  draw_pixel(pos.x  , pos.y-r, color);
  draw_pixel(pos.x+r, pos.y  , color);
  draw_pixel(pos.x-r, pos.y  , color);

  while (x<y) {
    if (f >= 0) {
      y--;
      ddF_y += 2;
      f += ddF_y;
    }
    x++;
    ddF_x += 2;
    f += ddF_x;
  
    draw_pixel(pos.x + x, pos.y + y, color);
    draw_pixel(pos.x - x, pos.y + y, color);
    draw_pixel(pos.x + x, pos.y - y, color);
    draw_pixel(pos.x - x, pos.y - y, color);
    draw_pixel(pos.x + y, pos.y + x, color);
    draw_pixel(pos.x - y, pos.y + x, color);
    draw_pixel(pos.x + y, pos.y - x, color);
    draw_pixel(pos.x - y, pos.y - x, color);
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

void graphicSystem::set_cursor(int16_t x, int16_t y) {
  cursor.x = x;
  cursor.y = y;
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
    draw_char_5x8(cursor.x, cursor.y, c, textcolor, textbgcolor, textsize);
    //_display->drawChar_5x8(cursor.x, cursor.y, c, textcolor, textbgcolor, textsize);
    cursor.x += textsize*6;
    if (wrap && (cursor.x > (_width - textsize*6))) {
      cursor.y += textsize*8;
      cursor.x = 0;
    }
  }
  return 1;
}

void graphicSystem::draw_char_5x8(int16_t x, int16_t y, unsigned char c, g_color_t color, g_color_t bg, uint8_t size) {
  if((x >= _width)            || // Clip right
     (y >= _height)           || // Clip bottom
     ((x + 6 * size - 1) < 0) || // Clip left
     ((y + 8 * size - 1) < 0))   // Clip top
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
  draw_picture(x, y, size*6, size*8, text_draw_buffer);
  //_display->draw_picture(pos.x, pos.y, text_draw_buffer, size*6, size*8);
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
          draw_pixel(x+i, y+j, color);
        else {  // big size
          fill_rect(x+(i*size), y+(j*size), size, size, color);
        } 
      } else if (bg != color) { // if bg == color, transparent background
        if (size == 1) // default size
          draw_pixel(x+i, y+j, bg);
        else {  // big size
          fill_rect(x+(i*size), y+(j*size), size, size, bg);
        }
      }
      line >>= 1;
    }
  }
#endif
}
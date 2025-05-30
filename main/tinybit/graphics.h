#ifndef graphics_H
#define graphics_H

// Screen dimensions
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 128

#include <stdint.h>

extern uint32_t fillColor;
extern uint32_t strokeColor;
extern int strokeWidth;

void set_fill_color(uint32_t);
void set_stroke_color(uint32_t);
void set_stroke_width(uint32_t);

void lua_setup_draw();

int millis();
int random_range(int, int);
void draw_sprite(int sourceX, int sourceY, int sourceW, int sourceH, int targetX, int targetY, int targetW, int targetH);
void draw_rect(int x, int y, int w, int h);
void draw_oval(int x, int y, int w, int h);
void set_stroke(int width, int r, int g, int b, int a);
void set_fill(int r, int g, int b, int a);
void draw_pixel(int x, int y);
void draw_cls();

#endif
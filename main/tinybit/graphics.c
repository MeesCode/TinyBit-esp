
#include <time.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>


#include "graphics.h"
#include "memory.h"

uint32_t fillColor = 0;
uint32_t fillColor2 = 0;
uint32_t strokeColor = 0;
int strokeWidth = 0;
uint16_t fillPattern = 0;

void blend(uint32_t* result, uint32_t fg, uint32_t bg) {
    float alpha_fg = (fg >> 24) / 255.0f;
    float alpha_bg = (bg >> 24) / 255.0f;
    float alpha_out = alpha_fg + alpha_bg * (1 - alpha_fg);

    *result = 0;

    if (alpha_out > 0) {
        *result = 
            ((uint8_t)((fg >> 16 & 0xff) * alpha_fg + (bg >> 16 & 0xff) * alpha_bg * (1 - alpha_fg) / alpha_out) << 16) |
            ((uint8_t)((fg >> 8 & 0xff) * alpha_fg + (bg >> 8 & 0xff) * alpha_bg * (1 - alpha_fg) / alpha_out) << 8) |
            ((uint8_t)((fg & 0xff) * alpha_fg + (bg & 0xff) * alpha_bg * (1 - alpha_fg) / alpha_out)) |
            ((uint32_t)(alpha_out * 255) << 24);
    }
}

int millis() {
    return clock() / (CLOCKS_PER_SEC / 1000);
}

int random_range(int min, int max) {
    return min + rand() / (RAND_MAX / (max - min + 1) + 1);
}

void draw_sprite(int sourceX, int sourceY, int sourceW, int sourceH, int targetX, int targetY, int targetW, int targetH) {


    float scaleX = (float)sourceW / (float)targetW;
    float scaleY = (float)sourceH / (float)targetH;

    for (int y = 0; y < targetH; ++y) {
        for (int x = 0; x < targetW; ++x) {

            if(x + targetX < 0 || x + targetX >= SCREEN_WIDTH || y + targetY < 0 || y + targetY >= SCREEN_HEIGHT) {
                continue;
            }

            int sourcePixelX = sourceX + (int)(x * scaleX);
            int sourcePixelY = sourceY + (int)(y * scaleY);

            uint32_t fillBackup = fillColor;
            fillColor = *(uint32_t*)&memory[MEM_SPRITESHEET_START + (sourcePixelY * SCREEN_WIDTH + sourcePixelX) * 4];
            draw_pixel(targetX + x, targetY + y);
            fillColor = fillBackup;
        }
    }
}

void draw_rect(int x, int y, int w, int h) {

    uint32_t fillBackup = fillColor;
    fillColor = strokeColor;

    for (int i = 0; i < strokeWidth; i++) {
        for (int j = 0; j < w; j++) {
            draw_pixel(x + j, y + i);
            draw_pixel(x + j, y + h - i - 1);
        }
        for (int j = 0; j < h; j++) {
            draw_pixel(x + i, y + j);
            draw_pixel(x + w - i - 1, y + j);
        }
    }

    fillColor = fillBackup;

    for (int i = strokeWidth; i < w - strokeWidth; i++) {
        for (int j = strokeWidth; j < h - strokeWidth; j++) {
            draw_pixel_fill(x + i, y + j);
        }
    }
}

// draw an oval with outline, specified by x, y, w, h
void draw_oval(int x, int y, int w, int h) {
    // Backup fill color
    uint32_t fillBackup = fillColor;

    float rx = w / 2.0f;
    float ry = h / 2.0f;

    float rx2 = rx * rx;
    float ry2 = ry * ry;

    float epsilon = 0.005f;  // Small value to fine-tune the boundary condition

    for (int i = 0; i < w; i++) {
        for (int j = 0; j < h; j++) {
            float dx = i - rx;
            float dy = j - ry;
            float d = (dx * dx) / rx2 + (dy * dy) / ry2;

            // Check if the point is inside the oval
            if (d <= 1.0f + epsilon) {
                // Check if the point is within the stroke area
                float strokeInnerRadius = (dx * dx) / ((rx - (float)strokeWidth - 0.1) * (rx - (float)strokeWidth - 0.1)) +
                    (dy * dy) / ((ry - (float)strokeWidth - 0.1) * (ry - (float)strokeWidth - 0.1));
                bool instroke = (strokeInnerRadius > 1.0f + epsilon);
                if (instroke) {
                    fillColor = instroke;
                    draw_pixel(x + i, y + j);
                }
                else {
                    fillColor = fillBackup;
                    draw_pixel_fill(x + i, y + j);
                }
            }
        }
    }

    fillColor = fillBackup;
}


void set_stroke(int width, int r, int g, int b, int a) {
    strokeWidth = width >= 0 ? width : 0;
    strokeColor = (r & 0xFF) | ((g & 0xFF) << 8) | ((b & 0xFF) << 16) | ((a & 0xFF) << 24);
}

void set_fill(int r, int g, int b, int a) {
    fillColor = (r & 0xFF) | ((g & 0xFF) << 8) | ((b & 0xFF) << 16) | ((a & 0xFF) << 24);
}

void set_fillp(int fillp) {
    fillPattern = fillp & 0xffff;
}

void set_fill2(int r, int g, int b, int a) {
    fillColor2 = (r & 0xFF) | ((g & 0xFF) << 8) | ((b & 0xFF) << 16) | ((a & 0xFF) << 24);
}

void draw_pixel(int x, int y) {
    if(x < 0 || x >= SCREEN_WIDTH || y < 0 || y >= SCREEN_HEIGHT) {
        return;
    }
    uint32_t* bg = (uint32_t*)&memory[MEM_DISPLAY_START + (x + (y * SCREEN_WIDTH)) * 4];
    blend(bg, fillColor, *bg);
}

void draw_pixel_fill(int x, int y) {
    uint32_t fillBackup = fillColor;
    if ((1 << ((x % 4) + (y % 4) * 4) & fillPattern)) {
        fillColor = fillColor2;
    }
    draw_pixel(x, y);
    fillColor = fillBackup;
}

void draw_cls() {
    memset(&memory[MEM_DISPLAY_START], 0, MEM_DISPLAY_SIZE);
}

void set_fill_pattern(uint16_t pattern) {
    fillPattern = pattern;
}

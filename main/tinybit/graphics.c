
#include <time.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>


#include "graphics.h"
#include "memory.h"

uint32_t fillColor = 0;
uint32_t strokeColor = 0;
int strokeWidth = 0;

void blend(uint32_t* result, uint32_t fg, uint32_t bg) {
    uint32_t alpha_fg = fg >> 24;
    
    if (alpha_fg == 255) {
        *result = fg;
        return;
    }
    if (alpha_fg == 0) {
        return;
    }
    
    uint32_t alpha_bg = bg >> 24;
    uint32_t inv_alpha = 255 - alpha_fg;
    
    uint32_t r = ((fg >> 16 & 0xff) * alpha_fg + (bg >> 16 & 0xff) * inv_alpha) / 255;
    uint32_t g = ((fg >> 8 & 0xff) * alpha_fg + (bg >> 8 & 0xff) * inv_alpha) / 255;
    uint32_t b = ((fg & 0xff) * alpha_fg + (bg & 0xff) * inv_alpha) / 255;
    uint32_t a = alpha_fg + (alpha_bg * inv_alpha) / 255;
    
    *result = (a << 24) | (r << 16) | (g << 8) | b;
}

int millis() {
    return clock() / (CLOCKS_PER_SEC / 1000);
}

int random_range(int min, int max) {
    return min + rand() / (RAND_MAX / (max - min + 1) + 1);
}

void draw_sprite(int sourceX, int sourceY, int sourceW, int sourceH, int targetX, int targetY, int targetW, int targetH) {
    int clipStartX = targetX < 0 ? -targetX : 0;
    int clipStartY = targetY < 0 ? -targetY : 0;
    int clipEndX = (targetX + targetW > SCREEN_WIDTH) ? SCREEN_WIDTH - targetX : targetW;
    int clipEndY = (targetY + targetH > SCREEN_HEIGHT) ? SCREEN_HEIGHT - targetY : targetH;
    
    if (clipStartX >= clipEndX || clipStartY >= clipEndY) return;
    
    int scaleX_fixed = (sourceW << 16) / targetW;
    int scaleY_fixed = (sourceH << 16) / targetH;
    
    uint32_t* dst = (uint32_t*)&memory[MEM_DISPLAY_START + ((targetY + clipStartY) * SCREEN_WIDTH + targetX + clipStartX) * 4];
    
    for (int y = clipStartY; y < clipEndY; ++y) {
        int sourcePixelY = sourceY + ((y * scaleY_fixed) >> 16);
        uint32_t* src_row = (uint32_t*)&memory[MEM_SPRITESHEET_START + sourcePixelY * SCREEN_WIDTH * 4];
        
        for (int x = clipStartX; x < clipEndX; ++x) {
            int sourcePixelX = sourceX + ((x * scaleX_fixed) >> 16);
            uint32_t srcColor = src_row[sourcePixelX];
            
            if ((srcColor >> 24) > 0) {
                blend(dst, srcColor, *dst);
            }
            dst++;
        }
        dst += SCREEN_WIDTH - (clipEndX - clipStartX);
    }
}

void draw_rect(int x, int y, int w, int h) {
    int clipX = x < 0 ? 0 : x;
    int clipY = y < 0 ? 0 : y;
    int clipW = (x + w > SCREEN_WIDTH) ? SCREEN_WIDTH - x : w;
    int clipH = (y + h > SCREEN_HEIGHT) ? SCREEN_HEIGHT - y : h;
    
    if (clipX >= SCREEN_WIDTH || clipY >= SCREEN_HEIGHT || clipW <= 0 || clipH <= 0) return;
    
    uint32_t* display = (uint32_t*)&memory[MEM_DISPLAY_START];
    
    if (strokeWidth > 0) {
        for (int i = 0; i < strokeWidth && i < clipH; i++) {
            uint32_t* top_row = &display[(clipY + i) * SCREEN_WIDTH + clipX];
            uint32_t* bottom_row = &display[(clipY + clipH - 1 - i) * SCREEN_WIDTH + clipX];
            
            for (int j = 0; j < clipW; j++) {
                blend(&top_row[j], strokeColor, top_row[j]);
                if (clipH - 1 - i != i) {
                    blend(&bottom_row[j], strokeColor, bottom_row[j]);
                }
            }
        }
        
        for (int j = strokeWidth; j < clipH - strokeWidth; j++) {
            for (int i = 0; i < strokeWidth && i < clipW; i++) {
                uint32_t* left_pixel = &display[(clipY + j) * SCREEN_WIDTH + clipX + i];
                uint32_t* right_pixel = &display[(clipY + j) * SCREEN_WIDTH + clipX + clipW - 1 - i];
                
                blend(left_pixel, strokeColor, *left_pixel);
                if (clipW - 1 - i != i) {
                    blend(right_pixel, strokeColor, *right_pixel);
                }
            }
        }
    }
    
    int fillStartX = strokeWidth;
    int fillStartY = strokeWidth;
    int fillW = clipW - 2 * strokeWidth;
    int fillH = clipH - 2 * strokeWidth;
    
    if (fillW > 0 && fillH > 0) {
        for (int j = 0; j < fillH; j++) {
            uint32_t* row = &display[(clipY + fillStartY + j) * SCREEN_WIDTH + clipX + fillStartX];
            for (int i = 0; i < fillW; i++) {
                blend(&row[i], fillColor, row[i]);
            }
        }
    }
}

// draw an oval with outline, specified by x, y, w, h
void draw_oval(int x, int y, int w, int h) {
    int rx = w >> 1;
    int ry = h >> 1;
    int rx2 = rx * rx;
    int ry2 = ry * ry;
    
    int strokeRx = rx - strokeWidth;
    int strokeRy = ry - strokeWidth;
    int strokeRx2 = strokeRx * strokeRx;
    int strokeRy2 = strokeRy * strokeRy;
    
    uint32_t* display = (uint32_t*)&memory[MEM_DISPLAY_START];
    
    for (int j = 0; j < h; j++) {
        int dy = j - ry;
        int dy2 = dy * dy;
        
        for (int i = 0; i < w; i++) {
            int px = x + i;
            int py = y + j;
            
            if (px < 0 || px >= SCREEN_WIDTH || py < 0 || py >= SCREEN_HEIGHT) continue;
            
            int dx = i - rx;
            int dx2 = dx * dx;
            
            int outer_test = dx2 * ry2 + dy2 * rx2;
            
            if (outer_test <= rx2 * ry2) {
                uint32_t* pixel = &display[py * SCREEN_WIDTH + px];
                
                if (strokeWidth > 0 && strokeRx > 0 && strokeRy > 0) {
                    int inner_test = dx2 * strokeRy2 + dy2 * strokeRx2;
                    if (inner_test > strokeRx2 * strokeRy2) {
                        blend(pixel, strokeColor, *pixel);
                    } else {
                        blend(pixel, fillColor, *pixel);
                    }
                } else {
                    blend(pixel, fillColor, *pixel);
                }
            }
        }
    }
}


void set_stroke(int width, int r, int g, int b, int a) {
    strokeWidth = width >= 0 ? width : 0;
    strokeColor = (r & 0xFF) | ((g & 0xFF) << 8) | ((b & 0xFF) << 16) | ((a & 0xFF) << 24);
}

void set_fill(int r, int g, int b, int a) {
    fillColor = (r & 0xFF) | ((g & 0xFF) << 8) | ((b & 0xFF) << 16) | ((a & 0xFF) << 24);
}

void draw_pixel(int x, int y) {
    if(x < 0 || x >= SCREEN_WIDTH || y < 0 || y >= SCREEN_HEIGHT) {
        return;
    }
    uint32_t* pixel = (uint32_t*)&memory[MEM_DISPLAY_START + (y * SCREEN_WIDTH + x) * 4];
    blend(pixel, fillColor, *pixel);
}

void draw_sprite_rotated(int sourceX, int sourceY, int sourceW, int sourceH, int targetX, int targetY, int targetW, int targetH, float angleDegrees) {
    float angleRad = angleDegrees * M_PI / 180.0f;
    float cosA = cosf(angleRad);
    float sinA = sinf(angleRad);
    
    float shear1 = -tanf(angleRad / 2.0f);
    float shear2 = sinA;
    float shear3 = shear1;
    
    int centerX = targetW / 2;
    int centerY = targetH / 2;
    
    int clipStartX = targetX < 0 ? -targetX : 0;
    int clipStartY = targetY < 0 ? -targetY : 0;
    int clipEndX = (targetX + targetW > SCREEN_WIDTH) ? SCREEN_WIDTH - targetX : targetW;
    int clipEndY = (targetY + targetH > SCREEN_HEIGHT) ? SCREEN_HEIGHT - targetY : targetH;
    
    if (clipStartX >= clipEndX || clipStartY >= clipEndY) return;
    
    int scaleX_fixed = (sourceW << 16) / targetW;
    int scaleY_fixed = (sourceH << 16) / targetH;
    
    for (int y = clipStartY; y < clipEndY; ++y) {
        for (int x = clipStartX; x < clipEndX; ++x) {
            int relX = x - centerX;
            int relY = y - centerY;
            
            float tempX = relX + shear1 * relY;
            float tempY = relY;
            
            tempX = tempX;
            tempY = tempY + shear2 * tempX;
            
            tempX = tempX + shear3 * tempY;
            tempY = tempY;
            
            int rotX = (int)(tempX + centerX);
            int rotY = (int)(tempY + centerY);
            
            if (rotX >= 0 && rotX < targetW && rotY >= 0 && rotY < targetH) {
                int sourcePixelX = sourceX + ((rotX * scaleX_fixed) >> 16);
                int sourcePixelY = sourceY + ((rotY * scaleY_fixed) >> 16);
                
                if (sourcePixelX >= sourceX && sourcePixelX < sourceX + sourceW &&
                    sourcePixelY >= sourceY && sourcePixelY < sourceY + sourceH) {
                    
                    uint32_t* src = (uint32_t*)&memory[MEM_SPRITESHEET_START + (sourcePixelY * SCREEN_WIDTH + sourcePixelX) * 4];
                    uint32_t srcColor = *src;
                    
                    if ((srcColor >> 24) > 0) {
                        uint32_t* dst = (uint32_t*)&memory[MEM_DISPLAY_START + ((targetY + y) * SCREEN_WIDTH + targetX + x) * 4];
                        blend(dst, srcColor, *dst);
                    }
                }
            }
        }
    }
}

void draw_cls() {
    memset(&memory[MEM_DISPLAY_START], 0, MEM_DISPLAY_SIZE);
}

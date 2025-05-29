#ifndef TINYBIT_H
#define TINYBIT_H

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

// cartridge dimensions
#define CARTRIDGE_WIDTH 336
#define CARTRIDGE_HEIGHT 376

void tinybit_init(uint8_t* cartridge_buffer, uint8_t* display_buffer);
void tinybit_frame();

#endif
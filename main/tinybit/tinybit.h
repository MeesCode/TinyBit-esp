#ifndef TINYBIT_H
#define TINYBIT_H

#include <stdint.h>
#include <stdbool.h>

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

// cartridge dimensions
#define CARTRIDGE_WIDTH 336
#define CARTRIDGE_HEIGHT 376

bool tinybit_init(uint8_t** display_buffer, uint8_t** button_state);
bool tinybit_feed_catridge(uint8_t* cartridge_buffer, size_t pixels);
bool tinybit_frame();
bool tinybit_start();

#endif
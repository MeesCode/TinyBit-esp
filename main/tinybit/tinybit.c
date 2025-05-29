#include "tinybit.h"

#include <inttypes.h>

#include "lua_functions.h"
#include "graphics.h"
#include "memory.h"
#include "audio.h"
#include "input.h"

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

int frame_timer = 0;
lua_State* L;

void tinybit_init(uint8_t* cartridge_buffer, uint8_t* display_buffer) {

    // init functions
    memory_init();

    // set up lua VM
    L = luaL_newstate();

    // load lua libraries
    static const luaL_Reg loadedlibs[] = {
        {LUA_GNAME, luaopen_base},
        {LUA_COLIBNAME, luaopen_coroutine},
        {LUA_TABLIBNAME, luaopen_table},
        {LUA_STRLIBNAME, luaopen_string},
        {LUA_MATHLIBNAME, luaopen_math},
        {NULL, NULL}
    };

    const luaL_Reg* lib;
    for (lib = loadedlibs; lib->func; lib++) {
        luaL_requiref(L, lib->name, lib->func, 1);
        lua_pop(L, 1);  /* remove lib */
    }

    // setup global variables
    lua_pushinteger(L, MEM_SIZE);
    lua_setglobal(L, "MEM_SIZE");
    lua_pushinteger(L, MEM_HEADER_START);
    lua_setglobal(L, "MEM_HEADER_START");
    lua_pushinteger(L, MEM_HEADER_SIZE);
    lua_setglobal(L, "MEM_HEADER_SIZE");
    lua_pushinteger(L, MEM_FONT_START);
    lua_setglobal(L, "MEM_FONT_START");
    lua_pushinteger(L, MEM_FONT_SIZE);
    lua_setglobal(L, "MEM_FONT_SIZE");
    lua_pushinteger(L, MEM_SPRITESHEET_START);
    lua_setglobal(L, "MEM_SPRITESHEET_START");
    lua_pushinteger(L, MEM_SPRITESHEET_SIZE);
    lua_setglobal(L, "MEM_SPRITESHEET_SIZE");
    lua_pushinteger(L, MEM_DISPLAY_START);
    lua_setglobal(L, "MEM_DISPLAY_START");
    lua_pushinteger(L, MEM_DISPLAY_SIZE);
    lua_setglobal(L, "MEM_DISPLAY_SIZE");
    // lua_pushinteger(L, MEM_USER_START);
    // lua_setglobal(L, "MEM_USER_START");

    // set lua tone variables
    lua_pushinteger(L, Ab);
    lua_setglobal(L, "Ab");
    lua_pushinteger(L, A);
    lua_setglobal(L, "A");
    lua_pushinteger(L, As);
    lua_setglobal(L, "As");
    lua_pushinteger(L, Bb);
    lua_setglobal(L, "Bb");
    lua_pushinteger(L, B);
    lua_setglobal(L, "B");
    lua_pushinteger(L, C);
    lua_setglobal(L, "C");
    lua_pushinteger(L, Cs);
    lua_setglobal(L, "Cs");
    lua_pushinteger(L, Db);
    lua_setglobal(L, "Db");
    lua_pushinteger(L, D);
    lua_setglobal(L, "D");
    lua_pushinteger(L, Ds);
    lua_setglobal(L, "Ds");
    lua_pushinteger(L, Eb);
    lua_setglobal(L, "Eb");
    lua_pushinteger(L, E);
    lua_setglobal(L, "E");
    lua_pushinteger(L, F);
    lua_setglobal(L, "F");
    lua_pushinteger(L, Fs);
    lua_setglobal(L, "Fs");
    lua_pushinteger(L, Gb);
    lua_setglobal(L, "Gb");
    lua_pushinteger(L, G);
    lua_setglobal(L, "G");
    lua_pushinteger(L, Gs);
    lua_setglobal(L, "Gs");

    // set lua waveforms
    lua_pushinteger(L, SINE);
    lua_setglobal(L, "SINE");
    lua_pushinteger(L, SAW);
    lua_setglobal(L, "SAW");
    lua_pushinteger(L, SQUARE);
    lua_setglobal(L, "SQUARE");

    lua_pushinteger(L, SCREEN_WIDTH);
    lua_setglobal(L, "SCREEN_WIDTH");
    lua_pushinteger(L, SCREEN_HEIGHT);
    lua_setglobal(L, "SCREEN_HEIGHT");

    // TODO: load font

    // uint8_t* spritesheet_buffer = (uint8_t*)malloc(SCREEN_WIDTH * SCREEN_HEIGHT * 4);
    char* source_buffer = (char*)malloc((CARTRIDGE_WIDTH * CARTRIDGE_HEIGHT * 4) - (SCREEN_WIDTH * SCREEN_HEIGHT * 4)); // max source size

    // iterate over cartridge buffer and extract spritesheet and source code
    for (int y = 0; y < CARTRIDGE_HEIGHT; y++) {
        for (int x = 0; x < CARTRIDGE_WIDTH; x++) {

            long pixel_index = ((y * CARTRIDGE_WIDTH) + x) * 4; // indicates pixel location in buffer
            long byte_index = ((y * CARTRIDGE_WIDTH) + x); // indicates which byte we are saving

            uint8_t r = cartridge_buffer[pixel_index];
            uint8_t g = cartridge_buffer[pixel_index + 1];
            uint8_t b = cartridge_buffer[pixel_index + 2];
            uint8_t a = cartridge_buffer[pixel_index + 3];

            // spritesheet data
            if (byte_index < SCREEN_WIDTH * SCREEN_HEIGHT * 4) {
                memory[MEM_SPRITESHEET_START + byte_index] = (r & 0x3) << 6 | (g & 0x3) << 4 | (b & 0x3) << 2 | (a & 0x3) << 0;
            }

            // source code
            else if (byte_index - SCREEN_WIDTH * SCREEN_HEIGHT * 4 < CARTRIDGE_WIDTH * CARTRIDGE_HEIGHT * 4) {
                source_buffer[byte_index - SCREEN_HEIGHT * SCREEN_WIDTH * 4] = (r & 0x3) << 6 | (g & 0x3) << 4 | (b & 0x3) << 2 | (a & 0x3) << 0;
            }
        }
    }
    
    // load lua file
    if (luaL_dostring(L, source_buffer) == LUA_OK) {
        lua_pop(L, lua_gettop(L));
    }
}

void tinybit_frame() {
    // perform lua draw function every frame
    lua_getglobal(L, "_draw");
    if (lua_pcall(L, 0, 1, 0) == LUA_OK) {
        lua_pop(L, lua_gettop(L));
    }

    // save current button state
    save_button_state();
}
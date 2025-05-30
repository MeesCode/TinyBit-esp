#include "tinybit.h"

#include <inttypes.h>
#include <stdbool.h>
#include <string.h>

#include "lua_functions.h"
#include "graphics.h"
#include "memory.h"
#include "audio.h"
#include "input.h"

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

size_t cartridge_index = 0; // index for cartridge buffer
//uint8_t source_buffer[CARTRIDGE_WIDTH * CARTRIDGE_HEIGHT * 4 - SCREEN_WIDTH * SCREEN_HEIGHT * 4]; // max source size
char source_buffer[4096];
int frame_timer = 0;
lua_State* L;

char error_message[256] = {0}; // error message buffer
uint8_t* tinybit_init() {

    strcpy(error_message, "lua error");

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

    lua_pushinteger(L, X);
	lua_setglobal(L, "X");
	lua_pushinteger(L, Z);
	lua_setglobal(L, "Z");
	lua_pushinteger(L, UP);
	lua_setglobal(L, "UP");
	lua_pushinteger(L, DOWN);
	lua_setglobal(L, "DOWN");
	lua_pushinteger(L, LEFT);
	lua_setglobal(L, "LEFT");
	lua_pushinteger(L, RIGHT);
	lua_setglobal(L, "RIGHT");
	lua_pushinteger(L, START);
	lua_setglobal(L, "START");

    lua_setup_functions(L);

    // TODO: load font

    return &memory[MEM_DISPLAY_START];
}

bool tinybit_feed_catridge(uint8_t* cartridge_buffer, size_t pixels){
    
    // TODO: fix this
    // check if cartridge index is within bounds
    // if (cartridge_index + (pixels/4) > (CARTRIDGE_WIDTH * CARTRIDGE_HEIGHT)/4) {
    //     return false; 
    // }

    for(int i = 0; i < pixels; i += 4) {
        uint8_t r = cartridge_buffer[i];
        uint8_t g = cartridge_buffer[i + 1];
        uint8_t b = cartridge_buffer[i + 2];
        uint8_t a = cartridge_buffer[i + 3];

        // spritesheet data
        if (cartridge_index < SCREEN_WIDTH * SCREEN_HEIGHT * 4) {
            memory[MEM_SPRITESHEET_START + cartridge_index] = (r & 0x3) << 6 | (g & 0x3) << 4 | (b & 0x3) << 2 | (a & 0x3) << 0;
        }

        // source code
        else if (cartridge_index - SCREEN_WIDTH * SCREEN_HEIGHT * 4 < CARTRIDGE_WIDTH * CARTRIDGE_HEIGHT * 4) {
            source_buffer[cartridge_index - SCREEN_HEIGHT * SCREEN_WIDTH * 4] = (r & 0x3) << 6 | (g & 0x3) << 4 | (b & 0x3) << 2 | (a & 0x3) << 0;
        }

        // increment cartridge index
        cartridge_index++;
    }

    return true;
}

bool tinybit_start(){
     // load lua file
    if (luaL_dostring(L, source_buffer) == LUA_OK) {
        lua_pop(L, lua_gettop(L));
    } else{
        return false; // error in lua code
    }

    return true;
}

bool tinybit_frame() {
    // perform lua draw function every frame
    lua_getglobal(L, "_draw");
    if (lua_pcall(L, 0, 1, 0) == LUA_OK) {
        lua_pop(L, lua_gettop(L));
    } else {
        return false; // error in lua code
    }

    // save current button state
    save_button_state();

    return true;
}
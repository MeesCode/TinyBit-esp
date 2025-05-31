#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>

#define MEM_SIZE 0x40000 // 512KiB
#define MEM_HEADER_START 0x0000
#define MEM_HEADER_SIZE 0x0500
#define MEM_FONT_START 0x0500
#define MEM_FONT_SIZE 0x0500
#define MEM_SPRITESHEET_START 0x10000
#define MEM_SPRITESHEET_SIZE 0x10000
#define MEM_DISPLAY_START 0x20000
#define MEM_DISPLAY_SIZE 0x10000
#define MEM_USER_START 0x30000
#define MEM_USER_SIZE 0x10000

extern uint8_t memory[MEM_SIZE];

void memory_init();
void mem_copy(int dst, int src, int size);
uint8_t mem_peek(int);
void mem_poke(int, int);

#endif
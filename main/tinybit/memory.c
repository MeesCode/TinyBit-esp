
#include <stdint.h>
#include <string.h>
#include "memory.h"


uint8_t memory[MEM_SIZE];

void memory_init() {
    memset(memory, 0, MEM_SIZE);
}

void mem_copy(int dst, int src, int size) {
    if (dst + size > MEM_SIZE || src + size > MEM_SIZE) {
        return;
    }
    memcpy(&memory[dst], &memory[src], size);
}

uint8_t mem_peek(int dst) {
    if (dst < 0 || dst > MEM_SIZE) {
        return 0;
    }
    return *(uint8_t*)&memory[dst];
}

void mem_poke(int dst, int val){
    if (dst < 0 || dst > MEM_SIZE) {
        return;
    }
    memory[dst] = val & 0xff;
}
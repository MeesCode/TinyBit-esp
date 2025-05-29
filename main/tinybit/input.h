#ifndef INPUT_H
#define INPUT_H

#include <stdbool.h>

typedef enum {
	X,
	Z,
	UP,
	DOWN,
	LEFT,
	RIGHT,
	START
} BUTTON;

extern bool prev_button_state[7];

void lua_setup_input();
void save_button_state();
bool input_btn(BUTTON btn);
bool input_btnp(BUTTON btn);


#endif

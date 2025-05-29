#include <stdint.h>

#include "graphics.h"
#include "memory.h"

int cursorX = 0;
int cursorY = 0;
const int fontWidth = 4;
const int fontHeight = 6;
uint32_t textColor = 0xffffffff;

char characters[16*8] = {
	'?', '"', '%', '\'', '(', ')', '*', '+', ',', '-', '.', '/', '!',  ' ', ' ', ' ',
	'0', '1', '2', '3', '4',  '5', '6', '7', '8', '9', ':', ';', '<', '=',  '>', '?',
	'@', 'a', 'b', 'c',  'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l',  'm', 'n', 'o',
	'p', 'q', 'r', 's',  't', 'u', 'v', 'w', 'x', 'y', 'z', '[', '\\', ']', '^', '_',
	'`', 'A', 'B', 'C',  'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L',  'M', 'N', 'O',
	'P', 'Q', 'R', 'S',  'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '{', '|',  '}', ' ', ' ',
	' ', ' ', ' ', ' ',  ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',  ' ', ' ', ' ',
	' ', ' ', ' ', ' ',  ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',  ' ', ' ', ' '
};

void font_text_color(int r, int g, int b, int a) {
	textColor = (r & 0xFF) | ((g & 0xFF) << 8) | ((b & 0xFF) << 16) | ((a & 0xFF) << 24);
}

void font_cursor(int x, int y) {
	cursorX = x;
	cursorY = y;
}

void font_prints(const char* str) {
	const char* ptr = str;
	int location = 0;

	int startX = cursorX;
	uint32_t fillBackup = fillColor;
	
	while (*ptr) {

		// process newline character
		if (*ptr == '\n') {
			cursorY += fontHeight;
			cursorX = startX;
			ptr++;
			continue;
		}

		// get character location
		for (int i = 0; i < 16 * 8; i++) {
			if (*ptr == characters[i]) location = i;
		}

		// draw character 
		for (int y = 0; y < fontHeight; y++) {
			for (int x = 0; x < fontWidth; x++) {
				uint8_t alpha = memory[(128 * 4 * 8 * (location / 16)) + 8 * 4 * (location % 16) + MEM_FONT_START + (x + (y * 128)) * 4 + 3];
				if (alpha != 0) {
					fillColor = textColor;
					fillColor = (fillColor & 0xffffff00) | alpha;
					draw_pixel(cursorX + x, cursorY + y);
				}
				else {
					fillColor = fillBackup;
					draw_pixel(cursorX + x, cursorY + y);
				}
			}
		}

		cursorX += fontWidth;
		ptr++;
	}

	fillColor = fillBackup;
}
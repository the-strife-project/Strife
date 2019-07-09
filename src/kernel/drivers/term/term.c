#include <kernel/drivers/term/term.h>
#include <kernel/paging/paging.h>
#include <kernel/drivers/term/font.h>

/*
	MODE is:
	0 for TTY
	1 for Graphics
*/
uint8_t term_mode = 0;

size_t term_width;
size_t term_height;

size_t term_row;
size_t term_column;
uint32_t term_bg;
uint32_t term_fg;

// This will be used in order to do some offsets in the font.
uint8_t lat1 = 0;

// This is whether or not the cursor is shown.
uint8_t cursor = 0;
uint8_t cursorIsInverted = 0;

void term_init(void) {
	term_mode = 0;
	term_setWidthHeight(TTY_WIDTH*8, TTY_HEIGHT*16);

	term_row = 0;
	term_column = 0;
	term_fg = vga_entry_color(7, 0);

	term_clear();
}

void term_goGraphics(uint32_t width, uint32_t height) {
	term_mode = 1;
	term_bg = 0x00000000;
	term_fg = 0x00BEBEBE;
	term_setWidthHeight(width, height);
	term_clear();
}

void term_setWidthHeight(uint32_t width, uint32_t height) {
	term_width = width/8;
	term_height = height/16;
}

void __term_putliteralchar(char c) {
	if(term_mode == 0) {
		// TTY
		TTY_putentryat(c, term_fg, term_column, term_row);
		return;
	}

	// Graphics
	for(int y=0; y<16; y++) {
		for(int x=0; x<8; x++) {
			uint32_t colorToWrite = term_bg;
			if(isBitSet(lat1, c, y, x)) colorToWrite = term_fg;

			VESA_putPixel(
				term_column*8+x,
				term_row*16+y,
				colorToWrite
			);
		}
	}

	if(++term_column == term_width) term_goDown();
	cursorIsInverted = 0;
}

void term_writec(char c) {
	switch(c) {
		case '\n':
			if(cursorIsInverted) blinkCursor();
			term_goDown();
			break;
		case '\xc2':
			// Latin-1
			lat1 = 2;
			break;
		case '\xc3':
			// Latin-1
			lat1 = 3;
			break;
		case '\t':
			for(int i=0; i<4; i++) __term_putliteralchar(' ');
			break;
		case '\b':
			term_left();
			for(int y=0; y<16; y++) {
				for(int x=0; x<8; x++) {
					VESA_putPixel(term_column*8+x, term_row*16+y, term_bg);
				}
			}
			break;
		default:
			__term_putliteralchar(c);
	}
}

void term_goDown() {
	term_column = 0;
	if(++term_row == term_height) term_row = 0;
	/* Scrolling should be implemented here. */
}

void term_goStart() {
	term_column = 0;
	term_row = 0;
}

void term_fill(uint32_t color) {
	term_goStart();
	term_bg = color;
	if(term_mode == 0) {
		// TTY
		for(unsigned int i=0; i<term_width*term_height; i++) {
			term_writec(' ');
		}
	} else {
		// VESA
		// This must be optimized!
		for(size_t y=0; y<(term_height+1)*16; y++) {
			for(size_t x=0; x<term_width*8; x++) {
				VESA_putPixel(x, y, term_bg);
			}
		}
	}
}

void term_clear() {
	term_fill(term_bg);
	term_goStart();
}

uint8_t term_getCurrentMode() { return term_mode; }
void term_setFGC(uint32_t color) { term_fg = color; }
void term_setBGC(uint32_t color) { term_bg = color; }
void term_left() {
	if(cursorIsInverted) blinkCursor();
	if(--term_column == 0xFFFFFFFF) {
		--term_row;
		term_column = term_width-1;
	}
}

void showCursor() { cursor = 1; }
void hideCursor() { cursor = 0; }

void blinkCursor() {
	if(!cursor) return;

	// Basically, invert the colors.
	for(int y=0; y<16; y++) {
		for(int x=0; x<8; x++) {
			size_t px = term_column*8+x;
			size_t py = term_row*16+y;
			if(VESA_getPixel(px, py) == term_fg) {
				VESA_putPixel(px, py, term_bg);
			} else {
				VESA_putPixel(px, py, term_fg);
			}
		}
	}

	cursorIsInverted = !cursorIsInverted;
}

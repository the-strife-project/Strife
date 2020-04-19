#include <kernel/drivers/term/term.hpp>
#include <kernel/paging/paging.hpp>
#include <kernel/drivers/term/font/font.hpp>
#include <kernel/drivers/term/cursor/cursor.hpp>
#include <kernel/klibc/stdlib.hpp>

size_t term_row;
size_t term_column;
uint8_t term_bg;
uint8_t term_fg;
uint16_t* TERM_BUFFER = (uint16_t*)0xB8000;

// This will be used in order to do some offsets in the font.
uint8_t lat1 = 0;
// This is for doing checks in '\n'.
uint8_t term_bg_cache;

void term_init(void) {
	term_row = 0;
	term_column = 0;
	term_clear();
}

void __term_putliteralchar(unsigned char c) {
	size_t index = term_row * TERM_WIDTH + term_column;
	TERM_BUFFER[index] = vga_entry(c, vga_entry_color(term_fg, term_bg));
	term_right();
}

void term_writec(unsigned char c) {
	switch(c) {
		case '\n':
			/*
				If the background color has changed since the last fill,
				we need to fill in the rest of the line with spaces.
			*/
			if(term_bg_cache != term_bg) {
				for(int i=term_column; i<TERM_WIDTH; ++i)
					__term_putliteralchar(' ');
				term_row--;
			}

			term_goDown();
			break;

		// Latin-1
		case 0xC2:
			lat1 = 2;
			break;
		case 0xC3:
			lat1 = 3;
			break;

		case '\t':
			// TODO: make this an actual tab with 8 spaces between each one.
			for(int i=0; i<4; i++) __term_putliteralchar(' ');
			break;
		case '\b':
			term_left();
			break;
		default:
			// Map C3 LAT-1 character where they should go. This depends on the font.
			if(c >= 0xA0 && lat1 == 3) __term_putliteralchar(c + 0x40);
			else __term_putliteralchar(c);
	}
}

void term_goDown() {
	term_column = 0;
	if(++term_row == TERM_HEIGHT) {
		// Time to scroll.
		term_row--;

		// Starting from row 1, move each character up.
		memcpy(TERM_BUFFER, TERM_BUFFER + TERM_WIDTH, sizeof(uint16_t)*TERM_WIDTH*(TERM_HEIGHT-1));

		// Finally, clear the last line.
		for(int i=0; i<TERM_WIDTH; i++)
			TERM_BUFFER[TERM_WIDTH*(TERM_HEIGHT-1) + i] = vga_entry(' ', vga_entry_color(term_fg, term_bg));
	}
}

void term_goStart() {
	term_column = 0;
	term_row = 0;
	updateCursor();
}

void term_fill(uint32_t color) {
	term_bg_cache = term_bg;
	term_bg = color;
	for(uint32_t i=0; i<TERM_WIDTH*TERM_HEIGHT; ++i)
		TERM_BUFFER[i] = vga_entry(' ', vga_entry_color(term_fg, term_bg));
}

void term_clear() {
	term_fill(term_bg);
	term_goStart();
}

void term_setFGC(uint8_t color) { term_fg = color; }
void term_setBGC(uint8_t color) { term_bg = color; }
void term_left() {
	if(--term_column == (size_t)~0) {
		--term_row;
		term_column = TERM_WIDTH-1;
	}
	updateCursor();
}
void term_right() {
	if(++term_column == TERM_WIDTH) term_goDown();
	updateCursor();
}

void showCursor() { CURSOR_enable(0, 15); }
void hideCursor() { CURSOR_disable(); }
void updateCursor() { CURSOR_update(term_column, term_row); }

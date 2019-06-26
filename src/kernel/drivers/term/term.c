#include <kernel/drivers/term/term.h>

/*
	MODE is:
	0 for TTY
	1 for VGA
*/
uint8_t term_mode = 0;


size_t term_row;
size_t term_column;
uint8_t term_color;

void term_init(void) {
	term_mode = 0;

	term_row = 0;
	term_column = 0;
	term_color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);

	term_clear();
}

void term_writec(char c) {
	switch(c) {
		case '\n':
			term_goDown();
			break;
		default:
			TTY_putentryat(c, term_color, term_column, term_row);
			if(++term_column == VGA_WIDTH) term_goDown();
	}
}

void term_goDown() {
	term_column = 0;
	if(++term_row == VGA_WIDTH) term_row = 0;
}

void term_goStart() {
	term_column = 0;
	term_row = 0;
}

void term_fill(uint8_t color) {
	term_goStart();
	term_color = color;
	for(unsigned int i=0; i<VGA_WIDTH*VGA_HEIGHT; i++) {
		term_writec(' ');
	}
}

void term_clear() {
	term_fill(term_color);
	term_goStart();
}

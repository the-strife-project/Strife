#include <kernel/drivers/term/term.h>
#include <kernel/paging/paging.h>
#include <kernel/drivers/term/font.h>

/*
	MODE is:
	0 for TTY
	1 for VGA
*/
uint8_t term_mode = 0;
uint32_t BIOS_font_addr;

size_t term_width;
size_t term_height;

size_t term_row;
size_t term_column;
uint8_t term_bg;
uint8_t term_fg;

void term_init(void) {
	term_mode = 0;
	term_setWidthHeight(TTY_WIDTH, TTY_HEIGHT);

	term_row = 0;
	term_column = 0;
	term_fg = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);

	term_clear();
}

void term_goVGA() {
	term_mode = 1;
	term_bg = VGA_COLOR_BLACK;
	term_fg = VGA_COLOR_LIGHT_GREY;
	term_setWidthHeight(40, 12);
	term_goStart();
}

void term_setWidthHeight(uint32_t width, uint32_t height) {
	term_width = width;
	term_height = height;
}

void __term_putliteralchar(char c) {
	if(term_mode == 0) {
		// TTY
		TTY_putentryat(c, term_fg, term_column, term_row);
	} else {
		// VGA
		for(int y=0; y<16; y++) {
			for(int x=0; x<8; x++) {
				uint8_t colorToWrite = term_bg;
				if(isBitSet(c, y, x)) colorToWrite = term_fg;

				VGA_putPixelRaw(
					term_column*8+x,
					term_row*16+y,
					colorToWrite
				);
			}
		}
	}

	if(++term_column == term_width) term_goDown();
}

void term_writec(char c) {
	switch(c) {
		case '\n':
			term_goDown();
			break;
		default:
			__term_putliteralchar(c);
	}
}

void term_goDown() {
	term_column = 0;
	if(++term_row == term_width) term_row = 0;
}

void term_goStart() {
	term_column = 0;
	term_row = 0;
}

void term_fill(uint8_t color) {
	term_goStart();
	term_bg = color;
	if(term_mode == 0) {
		// TTY
		for(unsigned int i=0; i<term_width*term_height; i++) {
			term_writec(' ');
		}
	} else {
		// VGA
		for(size_t y=0; y<(term_height+1)*16; y++) {
			for(size_t x=0; x<term_width*8; x++) {
				VGA_putPixelRaw(x, y, term_bg);
			}
		}
	}
}

void term_clear() {
	term_fill(term_bg);
	term_goStart();
}

#include <kernel/drivers/term/term.h>
#include <kernel/paging/paging.h>
#include <kernel/drivers/term/font/font.h>
#include <kernel/asm.h>
#include <kernel/drivers/term/cursor/cursor.h>

size_t term_row;
size_t term_column;
uint8_t term_bg;
uint8_t term_fg;
uint16_t* TERM_BUFFER = (uint16_t*)0xB8000;

// This will be used in order to do some offsets in the font.
uint8_t lat1 = 0;

void term_init(void) {
	term_row = 0;
	term_column = 0;

	/* --- LOAD VGA FONT --- */
	outw(0x03CE, 0x0005);	// Clear even/odd mode.
	outw(0x03CE, 0x0406);	// Map VGA memory to 0x0A0000.
	outw(0x03C4, 0x0402);	// Set bitplane 2.
	outw(0x03C4, 0x0604);	// Clear even/odd mode.

	// Copy the font. It's stored as 8x32, so we have to ommit the higher 16 bytes.
	// TODO: OPTIMIZE THIS
	unsigned char* LAT1_16 = getFont();
	unsigned char* vga_font = (unsigned char*)0x0A0000;
	for(int i=0; i<FONT_LENGTH/16; i++) {
		// Copy 16 bytes.
		for(int j=0; j<16; j++) *(vga_font++) = LAT1_16[i*16 + j];
		// Ignore 16 bytes.
		vga_font += 16;
	}

	// Restore VGA to normal operation.
	outw(0x03C4, 0x0302);
	outw(0x03C4, 0x0204);
	outw(0x03CE, 0x1005);
	outw(0x03CE, 0x0E06);

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
				Fill the rest of the line with spaces.
				This is useful in case the background color has changed
				since the last call to "term_clear()".

				TODO: avoid doing this all the time, keep a value of the
				background color and see if it has changed.
			*/
			for(int i=term_column; i<TERM_WIDTH; i++)
				__term_putliteralchar(' ');
			term_row--;

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
			for(int i=0; i<4; i++) __term_putliteralchar(' ');
			break;
		case '\b':
			term_left();
			break;
		default:
			// Map C3 LAT-1 character where they should go.
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
		// TODO: IMPROVE THIS ALGORITHM!!!
		for(int i=0; i<TERM_HEIGHT-1; i++) {
			for(int j=0; j<TERM_WIDTH; j++)
				TERM_BUFFER[i*TERM_WIDTH + j] = TERM_BUFFER[(i+1) * TERM_WIDTH + j];
		}

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
	term_bg = color;
	// TODO: This can be done way more efficient.
	for(unsigned int i=0; i<TERM_WIDTH*TERM_HEIGHT; i++)
		term_writec(' ');
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

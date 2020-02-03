#include <kernel/drivers/term/font/font.h>
#include <kernel/asm.h>
#include <klibc/stdlib.h>

uint8_t* font;

void setFont(uint8_t* f) {
	font = new uint8_t[FONT_LENGTH];
	for(uint16_t i=0; i<FONT_LENGTH; ++i)
		font[i] = f[i];
}

void loadFontToVGA() {
	outw(0x03CE, 0x0005);	// Clear even/odd mode.
	outw(0x03CE, 0x0406);	// Map VGA memory to 0x0A0000.
	outw(0x03C4, 0x0402);	// Set bitplane 2.
	outw(0x03C4, 0x0604);	// Clear even/odd mode.

	// Copy the font. It's stored as 8x32, so we have to ommit the higher 16 bytes.
	uint8_t* vga_font = (uint8_t*)0x0A0000;
	for(uint16_t i=0; i<FONT_LENGTH/16; i++) {
		// Copy 16 bytes.
		for(uint8_t j=0; j<16; j++) *(vga_font++) = font[i*16 + j];
		// Ignore 16 bytes.
		vga_font += 16;
	}

	// Restore VGA to normal operation.
	outw(0x03C4, 0x0302);
	outw(0x03C4, 0x0204);
	outw(0x03CE, 0x1005);
	outw(0x03CE, 0x0E06);
}

uint8_t* getFont() { return font; }

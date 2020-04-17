#ifndef TERM_H
#define TERM_H

#include <common/types.hpp>

static const uint8_t TERM_WIDTH = 80;
static const uint8_t TERM_HEIGHT = 25;

static inline uint8_t vga_entry_color(uint8_t fg, uint8_t bg) {
	return fg | bg << 4;
}
static inline uint16_t vga_entry(unsigned char uc, uint8_t color) {
	return (uint16_t) uc | (uint16_t) color << 8;
}

extern void load_BIOS_font(uint32_t fontBuffer);

void term_init();
void term_setWidthHeight(uint32_t width, uint32_t size);
void term_goGraphics(uint32_t width, uint32_t height);

void term_writec(unsigned char c);
void term_goDown(void);
void term_goStart(void);
void term_fill(uint32_t color);
void term_clear(void);

uint8_t term_getCurrentMode();
void term_setFGC(uint8_t color);
void term_setBGC(uint8_t color);
void term_left();
void term_right();

void showCursor();
void hideCursor();
void updateCursor();
#endif

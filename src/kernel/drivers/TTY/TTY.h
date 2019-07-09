#ifndef TTY_H
#define TTY_H

#include <common/types.h>

static const size_t TTY_WIDTH = 80;
static const size_t TTY_HEIGHT = 25;

static inline uint8_t vga_entry_color(uint8_t fg, uint8_t bg) {
	return fg | bg << 4;
}
static inline uint16_t vga_entry(unsigned char uc, uint8_t color) {
	return (uint16_t) uc | (uint16_t) color << 8;
}

void TTY_putentryat(char c, uint8_t color, size_t x, size_t y);

#endif

#include <kernel/drivers/TTY/TTY.h>

void TTY_putentryat(char c, uint8_t color, size_t x, size_t y) {
	const size_t index = y * VGA_WIDTH + x;

	uint16_t* term_buffer = (uint16_t*)0xB8000;
	term_buffer[index] = vga_entry(c, color);
}

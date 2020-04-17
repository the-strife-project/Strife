#include <kernel/drivers/term/cursor/cursor.hpp>
#include <kernel/asm.hpp>
#include <kernel/drivers/term/term.hpp>

void CURSOR_enable(uint8_t cursor_start, uint8_t cursor_end) {
	outb(0x3D4, 0x0A);
	outb(0x3D5, (inb(0x3D5) & 0xC0) | cursor_start);

	outb(0x3D4, 0x0B);
	outb(0x3D5, (inb(0x3D5) & 0xE0) | cursor_end);
}

void CURSOR_disable() {
	outb(0x3D4, 0x0A);
	outb(0x3D5, 0x20);
}

void CURSOR_update(uint8_t x, uint8_t y) {
	uint16_t pos = y * TERM_WIDTH + x;

	outb(0x3D4, 0x0F);
	outb(0x3D5, (uint8_t)(pos & 0xFF));
	outb(0x3D4, 0x0E);
	outb(0x3D5, (uint8_t)((pos >> 8) & 0xFF));
}

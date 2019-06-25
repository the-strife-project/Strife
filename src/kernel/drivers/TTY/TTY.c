#include <kernel/drivers/TTY/TTY.h>
#include <common/colors.h>

static inline uint8_t vga_entry_color(enum vga_color fg, enum vga_color bg) {
	return fg | bg << 4;
}
static inline uint16_t vga_entry(unsigned char uc, uint8_t color) {
	return (uint16_t) uc | (uint16_t) color << 8;
}

size_t terminal_row;
size_t terminal_column;
uint8_t terminal_color;
uint16_t* terminal_buffer;

void terminal_initialize(void) {
	terminal_row = 0;
	terminal_column = 0;
	terminal_color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
	terminal_buffer = (uint16_t*) 0xB8000;
	for(size_t y=0; y<VGA_HEIGHT; y++) {
		for(size_t x=0; x<VGA_WIDTH; x++) {
			const size_t index = y * VGA_WIDTH + x;
			terminal_buffer[index] = vga_entry(' ', terminal_color);
		}
	}
}

void terminal_putentryat(char c, uint8_t color, size_t x, size_t y) {
	const size_t index = y * VGA_WIDTH + x;
	terminal_buffer[index] = vga_entry(c, color);
}
void terminal_writec(char c) {
	terminal_putentryat(c, terminal_color, terminal_column, terminal_row);
	if(++terminal_column == VGA_WIDTH) terminal_goDown();
}

void terminal_goDown() {
	terminal_column = 0;
	if(++terminal_row == VGA_WIDTH) terminal_row = 0;
}

void terminal_goStart() {
	terminal_column = 0;
	terminal_row = 0;
}

void terminal_fill(uint8_t color) {
	terminal_goStart();
	terminal_color = color;
	for(unsigned int i=0; i<VGA_WIDTH*VGA_HEIGHT; i++) {
		terminal_writec(' ');
	}
}

void terminal_clear() {
	terminal_fill(terminal_color);
	terminal_goStart();
}

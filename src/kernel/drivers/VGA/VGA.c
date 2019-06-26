#include <kernel/drivers/VGA/VGA.h>
#include <kernel/asm.h>

/*
	This array is taken from here:
	https://files.osdev.org/mirrors/geezer/osd/graphics/modes.c
*/
unsigned char g_320x200x256[] = {
	/* MISC */
		0x63,
	/* SEQ */
		0x03, 0x01, 0x0F, 0x00, 0x0E,
	/* CRTC */
		0x5F, 0x4F, 0x50, 0x82, 0x54, 0x80, 0xBF, 0x1F,
		0x00, 0x41, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x9C, 0x0E, 0x8F, 0x28, 0x40, 0x96, 0xB9, 0xA3,
		0xFF,
	/* GC */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x05, 0x0F,
		0xFF,
	/* AC */
		0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
		0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
		0x41, 0x00, 0x0F, 0x00, 0x00
};

void VGA_writeRegisters(uint8_t* registers) {
	// Misc.
	outb(VGA_MISC_PORT, *(registers++));

	// Sequencer.
	for(uint8_t i=0; i<5; i++) {
		outb(VGA_SEQUENCER_INDEX_PORT, i);
		outb(VGA_SEQUENCER_DATA_PORT, *(registers++));
	}

	// CRT controller
	outb(VGA_CRTC_INDEX_PORT, 0x03);
	outb(VGA_CRTC_DATA_PORT, inb(VGA_CRTC_DATA_PORT) | 0x80);
	outb(VGA_CRTC_INDEX_PORT, 0x11);
	outb(VGA_CRTC_DATA_PORT, inb(VGA_CRTC_DATA_PORT) & ~0x80);

	registers[0x03] = registers[0x03] | 0x80;
	registers[0x11] = registers[0x11] & ~0x80;

	for(uint8_t i=0; i<25; i++) {
		outb(VGA_CRTC_INDEX_PORT, i);
		outb(VGA_CRTC_DATA_PORT, *(registers++));
	}

	// Graphics controller
	for(uint8_t i=0; i<9; i++) {
		outb(VGA_GRAPHICS_CONTROLLER_INDEX_PORT, i);
		outb(VGA_GRAPHICS_CONTROLLER_DATA_PORT, *(registers++));
	}

	// Attribute controller
	for(uint8_t i=0; i<21; i++) {
		inb(VGA_ATTR_CONTROLLER_RESET_PORT);
		outb(VGA_ATTR_CONTROLLER_INDEX_PORT, i);
		outb(VGA_ATTR_CONTROLLER_WRITE_PORT, *(registers++));
	}

	inb(VGA_ATTR_CONTROLLER_RESET_PORT);
	outb(VGA_ATTR_CONTROLLER_INDEX_PORT, 0x20);
}

uint8_t VGA_setMode(uint32_t width, uint32_t height, uint32_t colordepth) {
	if(width != VGA_WIDTH || height != VGA_HEIGHT || colordepth != VGA_COLORDEPTH) return 0;

	VGA_writeRegisters(g_320x200x256);
	return 1;
}

uint8_t* VGA_getFrameBufferSegment() {
	outb(VGA_GRAPHICS_CONTROLLER_INDEX_PORT, 0x06);
	uint8_t segmentNumber = inb(VGA_GRAPHICS_CONTROLLER_DATA_PORT) & (3 << 2);
	switch(segmentNumber) {
		default:
		case 0 << 2: return (uint8_t*)0x00000;
		case 1 << 2: return (uint8_t*)0xA0000;
		case 2 << 2: return (uint8_t*)0xB0000;
		case 3 << 2: return (uint8_t*)0xB8000;
	}
}

void VGA_putPixelRaw(uint32_t x, uint32_t y, uint8_t colorIndex) {
	uint8_t* pixelAddr = VGA_getFrameBufferSegment() + VGA_WIDTH*y + x;
	*pixelAddr = colorIndex;
}

#ifndef VESA_H
#define VESA_H

/*
	Thanks a lot to this tutorial:
	https://wiki.osdev.org/User:Omarrx024/VESA_Tutorial
*/

#include <common/types.h>
#include <kernel/V86/V86.h>

#define VESA_INT 0x10
#define VESA_GET_INFO 0x4F00
#define VESA_GET_MODE_INFO 0x4F01
#define VESA_SET_MODE 0x4F02

struct VBE_info_structure {
	char signature[4];	// Should be VESA if everything worked.
	uint16_t version;
	uint32_t oem;	// segment:offset pointer to OEM
	uint32_t capabilities;	// Card capabilities
	uint32_t video_modes;	// segment:offset pointer to list of supported video modes
	uint16_t video_memory;	// Amount of video memory in 64KB blocks.
	uint16_t software_rev;
	uint32_t vendor;	// segment:offset to card vendor string
	uint32_t product_name;	// segment:offset to card model name
	uint32_t product_rev;
	char reserved[222];
	char oem_data[256];
} __attribute__((packed));


struct VBE_mode_all_info {
	uint16_t attributes;	// Bit 7 indicates that the mode supports linear frame buffer.
	uint8_t window_a;
	uint8_t window_b;
	uint16_t granularity;
	uint16_t window_size;
	uint16_t segment_a;
	uint16_t segment_b;
	uint32_t win_func_ptr;
	uint16_t pitch;
	uint16_t width;			// width in pixels
	uint16_t height;			// height in pixels
	uint8_t w_char;
	uint8_t y_char;
	uint8_t planes;
	uint8_t bpp;			// bits per pixel in this mode
	uint8_t banks;
	uint8_t memory_model;
	uint8_t bank_size;
	uint8_t image_pages;
	uint8_t reserved0;

	uint8_t red_mask;
	uint8_t red_position;
	uint8_t green_mask;
	uint8_t green_position;
	uint8_t blue_mask;
	uint8_t blue_position;
	uint8_t reserved_mask;
	uint8_t reserved_position;
	uint8_t direct_color_attributes;

	uint32_t framebuffer;		// physical address of the linear frame buffer; write here to draw to the screen
	uint32_t off_screen_mem_off;
	uint16_t off_screen_mem_size;	// size of memory in the framebuffer but not being displayed on the screen
	uint8_t reserved1[206];
} __attribute__((packed));

struct VBE_mode_info {
	uint16_t attributes;
	uint16_t width;
	uint16_t height;
	uint8_t bpp;
	uint32_t framebuffer;
};


uint16_t* VESA_getModes();
struct VBE_mode_info* VESA_getModeInfo(uint16_t mode);
void VESA_setMode(uint16_t mode);

void VESA_init(uint16_t width, uint16_t height, uint8_t colordepth);
void VESA_putPixel(uint16_t x, uint16_t y, uint32_t color);
uint32_t VESA_getPixel(uint16_t x, uint16_t y);

#endif

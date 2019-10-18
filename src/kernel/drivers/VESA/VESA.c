#include <kernel/drivers/VESA/VESA.h>
#include <kernel/drivers/PIC/PIC.h>
#include <klibc/stdio.h>
#include <klibc/stdlib.h>
#include <kernel/kernel_panic/kernel_panic.h>
#include <kernel/paging/paging.h>

uint16_t VESA_currentMode = 0xFFFF;
uint16_t VESA_currentMode_width = 0;
uint16_t VESA_currentMode_height = 0;
uint8_t VESA_currentMode_bpp = 0;
uint32_t VESA_framebuffer = 0;

uint16_t* VESA_getModes() {
	/*
		The output of the BIOS interrupt will be stored at 0x7E00.
		It is marked as free to use here:
		https://wiki.osdev.org/Memory_Map_(x86)
	*/

	char* info_addr = (char*)0x7E00;
	strcpy(info_addr, "VBE2");
	for(int i=4; i<512; i++) *(info_addr+i) = 0;

	struct regs16_t regs;
	regs.ax = VESA_GET_INFO;
	regs.es = 0;	// Output segment.
	regs.di = 0x7E00;	// Output offset.
	V86(VESA_INT, &regs);
	pic_set_mask();

	struct VBE_info_structure* info = (struct VBE_info_structure*)info_addr;

	// Check VBE2 is supported.
	if(strcmp(info->signature, "VESA") != 0) kernel_panic(3);	// VBE2 not supported.

	// Get the number of modes.
	int numberOfModes = 1;	// Count 0xFFFF.
	for(uint16_t* p=(uint16_t*)info->video_modes; *p != 0xFFFF; p++)
		numberOfModes++;

	// Save 'em.
	uint16_t* ret = jmalloc(sizeof(uint16_t) * numberOfModes);
	for(int i=0; i<numberOfModes; i++)
		ret[i] = ((uint16_t*)info->video_modes)[i];

	return ret;
}

struct VBE_mode_info* VESA_getModeInfo(uint16_t mode) {
	struct regs16_t regs;
	regs.ax = VESA_GET_MODE_INFO;
	regs.cx = mode;
	regs.es = 0;	// Output segment.
	regs.di = 0x7E00;	// Output offset.
	V86(VESA_INT, &regs);
	pic_set_mask();

	struct VBE_mode_all_info* aux = (struct VBE_mode_all_info*)0x7E00;

	// Save the important data.
	struct VBE_mode_info* ret = (struct VBE_mode_info*)jmalloc(sizeof(struct VBE_mode_info));
	ret->attributes = aux->attributes;
	ret->width = aux->width;
	ret->height = aux->height;
	ret->bpp = aux->bpp;
	ret->framebuffer = aux->framebuffer;

	return ret;
}

void VESA_setMode(uint16_t mode) {
	struct regs16_t regs;
	regs.ax = VESA_SET_MODE;
	regs.bx = mode;
	// Enable linear framebuffer.
	regs.bx |= 0b0100000000000000;
	// Set 15th bit to zero.
	regs.bx &= 0b0111111111111111;

	V86(VESA_INT, &regs);
	pic_set_mask();
}

// Returns framebuffer.
void VESA_init(uint16_t width, uint16_t height, uint8_t colordepth) {
	uint16_t* modes = VESA_getModes();

	for(uint16_t* i=modes; *i != 0xFFFF; i++) {
		struct VBE_mode_info* info = VESA_getModeInfo(*i);

		// Check if it supports linear frame buffer.
		if(!(info->attributes & 0b0000000010000000)) {
			// It doesn't support it. Go to the next one.
			continue;
		}

		if(info->width == width && info->height == height && info->bpp == colordepth) {
			VESA_currentMode = *i;
			VESA_currentMode_width = width;
			VESA_currentMode_height = height;
			VESA_currentMode_bpp = colordepth;
			VESA_framebuffer = info->framebuffer;

			// Map the pages of the framebuffer.
			uint32_t fb_psize = width * height * (colordepth >> 3);
			for(uint32_t z=0; z<fb_psize; z+=4096)
				paging_mapPage(
					VESA_framebuffer + z,
					VESA_framebuffer + z,
					PT_PRESENT | PT_RW | PT_USED
				);

			jfree(info);
			break;
		}

		jfree(info);
	}
	jfree(modes);

	/*
		If the mode could not be found, kernel panic.
		In the future this would have to be changed into falling into another mode
		with fewer colors.
	*/
	if(VESA_currentMode == 0xFFFF) kernel_panic(4);

	// Set the mode.
	VESA_setMode(VESA_currentMode);
}

void VESA_putPixel(uint16_t x, uint16_t y, uint32_t color) {
	uint32_t pixel = y * VESA_currentMode_width * VESA_currentMode_bpp/8;
	pixel += x * VESA_currentMode_bpp/8;
	pixel += VESA_framebuffer;
	*((uint32_t*)pixel) = color;
}

uint32_t VESA_getPixel(uint16_t x, uint16_t y) {
	uint32_t pixel = y * VESA_currentMode_width * VESA_currentMode_bpp/8;
	pixel += x * VESA_currentMode_bpp/8;
	pixel += VESA_framebuffer;
	return *((uint32_t*)pixel);
}

/*
	A neat splash for the boot.
*/
#include <kernel/splash.h>
#include <kernel/drivers/term/font.h>
#include <kernel/drivers/term/term.h>
#include <klibc/stdlib.h>

uint32_t __getGradient(uint32_t origin, uint32_t end, int maxstep, int step) {
	uint8_t origin_r = (origin & 0x00FF0000) >> 16;
	uint8_t origin_g = (origin & 0x0000FF00) >> 8;
	uint8_t origin_b = origin & 0x000000FF;

	uint8_t end_r = (end & 0x00FF0000) >> 16;
	uint8_t end_g = (end & 0x0000FF00) >> 8;
	uint8_t end_b = end & 0x000000FF;

	uint8_t ret_r, ret_g, ret_b;
	if(origin_r <= end_r) ret_r = end_r - origin_r;
	else ret_r = origin_r - end_r;
	if(origin_g <= end_g) ret_g = end_g - origin_g;
	else ret_g = origin_g - end_g;
	if(origin_b <= end_b) ret_b = end_b - origin_b;
	else ret_b = origin_b - end_b;

	ret_r /= maxstep; ret_g /= maxstep; ret_b /= maxstep;
	ret_r *= step; ret_g *= step; ret_b *= step;

	if(origin_r <= end_r) ret_r = origin_r + ret_r;
	else ret_r = origin_r - ret_r;
	if(origin_g <= end_g) ret_g = origin_g + ret_g;
	else ret_g = origin_g - ret_g;
	if(origin_b <= end_b) ret_b = origin_b + ret_b;
	else ret_b = origin_b - ret_b;

	uint32_t ret;
	ret = ret_r << 16;
	ret |= ret_g << 8;
	ret |= ret_b;
	return ret;
}

void showSplash(char* toWrite, int splashSize, int marginx, int marginy) {
	// For each character.
	for(size_t i=0; i<strlen(toWrite); i++) {
		// (8x16 dimensions)
		for(int y=0; y<16; y++) {
			for(int x=0; x<8; x++) {
				// Check if the bit is set. We assume the background is already set.
				if(!isBitSet(0, toWrite[i], y, x)) continue;

				// Paint 'splashSize' pixels instead of one. This way we get big letters.
				for(int ky=0; ky<splashSize; ky++) {
					for(int kx=0; kx<splashSize; kx++) {
						VESA_putPixel(
							marginx + i*8*splashSize+x*splashSize+kx,
							marginy + y*splashSize+ky,
							__getGradient(
								0x00FFFF00,
								0x0000FF00,
								5*8*splashSize,
								i*8*splashSize+x*splashSize+kx
							)
						);
					}
				}
			}
		}
	}

	for(int i=0; i<2*splashSize; i++) term_goDown();
}

#ifndef TERM_H
#define TERM_H

#include <kernel/drivers/TTY/TTY.h>
#include <kernel/drivers/VGA/VGA.h>

/*
	This is basically a wrapper around TTY and VGA, so it's an
	abstraction layer for printing text to screen, no matter what method.
*/

extern void load_BIOS_font(uint32_t fontBuffer);

void term_init();
void term_setWidthHeight(uint32_t width, uint32_t size);
void term_goVGA();

void term_writec(char c);
void term_goDown(void);
void term_goStart(void);
void term_fill(uint8_t color);
void term_clear(void);

#endif

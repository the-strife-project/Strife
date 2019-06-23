#ifndef TTY_H
#define TTY_H

#include <common/types.h>

static const size_t VGA_WIDTH = 80;
static const size_t VGA_HEIGHT = 25;

void terminal_initialize(void);
void terminal_putentryat(char c, uint8_t color, size_t x, size_t y);
void terminal_writec(char c);
void terminal_goDown(void);
void terminal_goStart(void);
void terminal_fill(uint8_t color);
void terminal_clear(void);

#endif

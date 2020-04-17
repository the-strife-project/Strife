#ifndef CURSOR_H
#define CURSOR_H

// Thanks to: https://wiki.osdev.org/Text_Mode_Cursor

#include <common/types.hpp>

void CURSOR_enable(uint8_t cursor_start, uint8_t cursor_end);
void CURSOR_disable();
void CURSOR_update(uint8_t x, uint8_t y);

#endif

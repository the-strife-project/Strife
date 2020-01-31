#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <common/types.h>
#include <klibc/STL/window>

#define KEYBOARD_IDT_ENTRY 0x21
#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_STATUS_PORT 0x64

window<char>& keyboard_getBuffer();
bool keyboard_returnPressed();

extern "C" void IDT_keyboard(void);
extern "C" void keyboard_handler(void);

void keyboard_init(void);

void keyboard_pause(void);
void keyboard_resume(bool show_);

#endif

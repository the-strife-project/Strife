#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <common/types.hpp>
#include <klibc/STL/window>
#include <klibc/string>

#define KEYBOARD_IDT_ENTRY 0x21
#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_STATUS_PORT 0x64

// TODO: MAKE THIS A CLASS WITH A GLOBAL INSTANCE

window<char>& keyboard_getBuffer();
bool keyboard_returnPressed();

extern "C" void IDT_keyboard(void);
extern "C" void keyboard_handler(void);

void keyboard_init();
void keyboard_pause();
void keyboard_resume(bool);

// There's no need for unset silentmode. It gets unset when a key is pressed.
void keyboard_setSilentMode();
bool keyboard_getSilentMode();
const string& keyboard_getSilentPressed();

#endif

#ifndef KEYBOARD_H
#define KEYBOARD_H

#define KEYBOARD_IDT_ENTRY 0x21
#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_STATUS_PORT 0x64

extern void IDT_keyboard(void);
void keyboard_handler(void);

void keyboard_init(void);

void keyboard_pause(void);
void keyboard_resume(void);

#endif

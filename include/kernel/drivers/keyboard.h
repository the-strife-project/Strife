#ifndef KEYBOARD_H
#define KEYBOARD_H

#define KEYBOARD_IDT_ENTRY 0x21

extern void IDT_keyboard(void);
void keyboard_handler(void);

void keyboard_init(void);

void keyboard_pause(void);
void keyboard_resume(void);

#endif

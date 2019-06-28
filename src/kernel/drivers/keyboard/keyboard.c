#include <kernel/drivers/keyboard/keyboard.h>
#include <libc/stdio.h>
#include <kernel/asm.h>
#include <kernel/GDT/GDT.h>
#include <kernel/PIC/PIC.h>
#include <kernel/IDT/IDT.h>
#include <kernel/kernel_panic/kernel_panic.h>
#include <kernel/drivers/keyboard/kb_layout.h>

/*
	'differentiate' marks whether the first byte of the interrupt
	is 0xE0. This is used to distinguish some keys with the same keycode.
	The rest should require no further explanation.
*/
uint8_t differentiate = 0;
uint8_t altgr = 0;
uint8_t Lshift = 0;
uint8_t Rshift = 0;
uint8_t numb = 0;
uint8_t mayb = 0;

uint8_t accent0 = 0;	// ^
uint8_t accent1 = 0;	// `
uint8_t accent2 = 0;	// ´
uint8_t accent3 = 0;	// ¨

uint8_t ctrl =  0;
uint8_t alt = 0;

// 'bk' is for 'bufferKey'.
void bk(char* a) {
	printf(a);
}

void keyboard_handler(void) {
	outb(PIC_IO_PIC1, PIC_EOI);

	uint8_t status = inb(KEYBOARD_STATUS_PORT);

	if(!(status & 0x01)) {
		// Lowest bit is NOT set. The buffer is empty.
		return;
	}

	uint8_t keycode = inb(KEYBOARD_DATA_PORT);

	if(keycode == 0xE0) differentiate = 1;
	//printf("%x ", keycode); return;

	char* handle = 0;
	// Check first some special cases.
	switch(keycode) {
		case 0x38:
			// Altgr down
			if(differentiate) {
				altgr = 1;
				handle = (char*)1;
			}
			break;
		case 0xB8:
			// Altgr up
			if(differentiate) {
				altgr = 0;
				handle = (char*)1;
			}
			break;
		case 0x2A:
			// Left shift down
			Lshift = 1;
			break;
		case 0xAA:
			// Left shift up
			Lshift = 0;
			break;
		case 0x36:
			// Right shift down
			Rshift = 1;
			break;
		case 0xB6:
			// Right shift up
			Rshift = 0;
			break;
		case 0x35:
			/*
				This charcode is duplicated.
				'differentiate' indicates whether it's a '-'
				or a '/'.
			*/
			handle = (char*)1;
			if(differentiate) bk("/");
			else {
				if(Lshift || Rshift) bk("_");
				else bk("-");
			}
			break;
		case 0x0E:
			// Backspace.
			handle = (char*)1;
			bk("\b");
			break;
		case 0x3A:
			// Mayus block
			handle = (char*)1;
			mayb = !mayb;
			break;
		case 0x45:
			// Num block
			handle = (char*)1;
			numb = !numb;
			break;
		case 0x1A:
			// `
			if(altgr) break;
			handle = (char*)1;
			if(Lshift || Rshift) {
				// ^
				if(accent0 || accent1 || accent2 || accent3) {
					bk("^");
					accent0 = accent1 = accent2 = accent3 = 0;
				} else {
					accent0 = 1;
				}
			} else {
				// `
				if(accent0 || accent1 || accent2 || accent3) {
					bk("`");
					accent0 = accent1 = accent2 = accent3 = 0;
				} else {
					accent1 = 1;
				}
			}
			break;
		case 0x28:
			// ´
			if(altgr) break;
			handle = (char*)1;
			if(Lshift || Rshift) {
				// ¨
				if(accent0 || accent1 || accent2 || accent3) {
					bk("¨");
					accent0 = accent1 = accent2 = accent3 = 0;
				} else {
					accent3 = 1;
				}
			} else {
				// ´
				if(accent0 || accent1 || accent2 || accent3) {
					bk("´");
					accent0 = accent1 = accent2 = accent3 = 0;
				} else {
					accent2 = 1;
				}
			}
			break;
	}
	if(handle) return;

	if(keycode >= 128) return;	// Key release.


	if(altgr)
		handle = KB_LAYOUT_ALTGR[keycode];
	else if(Lshift || Rshift || mayb)
		if(accent0) handle = KB_LAYOUT_SHIFT_AC0[keycode];
		else if(accent1) handle = KB_LAYOUT_SHIFT_AC1[keycode];
		else if(accent2) handle = KB_LAYOUT_SHIFT_AC2[keycode];
		else if(accent3) handle = KB_LAYOUT_SHIFT_AC3[keycode];
		else handle = KB_LAYOUT_SHIFT[keycode];
	else {
		if(accent0) handle = KB_LAYOUT_AC0[keycode];
		else if(accent1) handle = KB_LAYOUT_AC1[keycode];
		else if(accent2) handle = KB_LAYOUT_AC2[keycode];
		else if(accent3) handle = KB_LAYOUT_AC3[keycode];
		else if(numb) handle = KB_LAYOUT_NUM[keycode];
		else handle = KB_LAYOUT_NONUM[keycode];
	}

	if(!handle) return;

	bk(handle);
	differentiate = 0;
	accent0 = accent1 = accent2 = accent3 = 0;
}

void keyboard_init(void) {
	IDT_SET_ENT(IDT[KEYBOARD_IDT_ENTRY], 0, _KERNEL_CODESEGMENT, (uint32_t)IDT_keyboard, 0);
	keyboard_resume();

	// Discard already pressed keys.
	inb(KEYBOARD_DATA_PORT);
	inb(KEYBOARD_STATUS_PORT);
}

void keyboard_pause(void) {
	pic_disable_irq(1);
}
void keyboard_resume(void) {
	pic_enable_irq(1);
}

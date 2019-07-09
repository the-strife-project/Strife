#include <kernel/drivers/keyboard/keyboard.h>
#include <libc/stdio.h>
#include <libc/stdlib.h>
#include <kernel/asm.h>
#include <kernel/GDT/GDT.h>
#include <kernel/drivers/PIC/PIC.h>
#include <kernel/IDT/IDT.h>
#include <kernel/kernel_panic/kernel_panic.h>
#include <kernel/drivers/keyboard/kb_layout.h>
#include <kernel/drivers/term/term.h>

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

/*
	The keyboard buffer will be stored at 0x7E00.
	There are more than 400KiB free there.
*/
char* buffer = (char*)0x7E00;
int buffered;
int __kbcursor;
uint8_t show;
void bk(char* a) {	// Buffer key
	if(strcmp(a, "\b") == 0) {
		// If the buffer is empty, don't do anything.
		if(buffered && __kbcursor) {
			// Move characters at the right one position to the left.
			char* baux = jmalloc(buffered);	// backspace auxiliar
			strcpy(baux, buffer+__kbcursor);
			term_left();
			printf("%s ", baux);

			// Move the cursor back.
			for(size_t i=0; i<strlen(baux)+1; i++) {
				if(baux[i] == '\xc2' || baux[i] == '\xc3') continue;
				term_left();
			}

			// Overwrite the buffer.
			int diff = 1;
			if(buffer[__kbcursor-2] == '\xc2' || buffer[__kbcursor-2] == '\xc3') diff++;
			strcpy(buffer + __kbcursor - diff, baux);
			jfree(baux);

			// Pull the counters back.
			if(diff == 2) {
				buffered--; __kbcursor--;
			}
			buffer[--buffered] = 0;
			__kbcursor--;
		}
		return;
	} else if(strcmp(a, "[LEFT]") == 0) {
		// Similar to above.
		if(__kbcursor) {
			term_left();
			__kbcursor--;
			if(buffer[__kbcursor-1] == '\xc2' || buffer[__kbcursor-1] == '\xc3') {
				__kbcursor--;
			}
		}
		return;
	}
	// Print the new characters.
	char* aux = jmalloc(buffered+strlen(a));	// Quick and dirty.
	strcpy(aux, a);
	strcat(aux, buffer+__kbcursor);
	if(show) printf("%s", aux);

	// Move the cursor back as many positions as printable character 'aux' has.
	for(size_t i=0; i<strlen(aux)-strlen(a); i++) {
		if(aux[i+strlen(a)] == '\xc2' || aux[i+strlen(a)] == '\xc3') continue;
		term_left();
	}

	// Update the buffer.
	strcpy(buffer+__kbcursor, aux);
	jfree(aux);

	buffered += strlen(a);
	__kbcursor += strlen(a);
}
char* keyboard_getBuffer() { return buffer; }
int keyboard_getBuffered() { return buffered; }

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
		case 0x4B:
			// <-
			if(!differentiate) break;
			handle = (char*)1;
			bk("[LEFT]");
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

	// Discard already pressed keys.
	inb(KEYBOARD_DATA_PORT);
	inb(KEYBOARD_STATUS_PORT);
}

void keyboard_pause(void) {
	pic_disable_irq(1);
}
void keyboard_resume(uint8_t show_) {
	// Reset the buffer.
	*buffer = 0;
	buffered = 0;
	__kbcursor = 0;
	show = show_;
	pic_enable_irq(1);
}

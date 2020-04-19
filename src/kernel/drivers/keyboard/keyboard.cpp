#include <kernel/drivers/keyboard/keyboard.hpp>
#include <kernel/klibc/stdio>
#include <kernel/klibc/stdlib.hpp>
#include <kernel/asm.hpp>
#include <kernel/GDT/GDT.hpp>
#include <kernel/drivers/PIC/PIC.hpp>
#include <kernel/IDT/IDT.hpp>
#include <kernel/kernel_panic/kernel_panic.hpp>
#include <kernel/drivers/keyboard/kb_layout.hpp>
#include <kernel/drivers/term/term.hpp>

#define IS_LATIN1(x) ((x == '\xC2') || (x == '\xC3'))

/*
	'differentiate' marks whether the first byte of the interrupt
	is 0xE0. This is used to distinguish some keys with the same keycode.
	The rest should require no further explanation.
*/
bool differentiate = false;
bool altgr = false;
bool Lshift = false;
bool Rshift = false;
bool mayb = false;	// Mayus block

bool accent0 = false;	// ^
bool accent1 = false;	// `
bool accent2 = false;	// ´
bool accent3 = false;	// ¨

// These are not implemented at the moment.
bool ctrl = false;
bool alt = false;

/*
	The idea: use a window to save the characters.
	Insertion, deletion, moving back, and forward, are all O(1),
	so it fits perfectly for this case.
*/
window<char> buffer;
bool returnPressed = false;
bool show = true;

/*
	Silent Mode is for getch().
	Just registers the last key pressed, and does not
	execute the corresponding routine.
*/
bool silentMode = false;
string silentPressed;	// TODO: This should be an ID for immediate comparisons.

// Function to print the characters at the right of the cursor.
// extraSpace is in case we're deleting a character.
void printRight(bool extraSpace) {
	if(show) {
		size_t count = 0;
		for(auto const& x : buffer) {
			printf("%c", x);
			++count;
		}

		if(extraSpace) {
			printf(" ");
			term_left();
		}

		for(size_t i=0; i<count; ++i)
			term_left();
	}
}

void bk(const string& a) {	// Buffer key
	// If we're on Silent Mode, we're done.
	if(silentMode) {
		silentMode = false;
		silentPressed = a;
		return;
	}

	if(a == "\b") {
		// If the buffer is empty, don't do anything.
		if(buffer.isBeginning()) return;

		// Move the cursor back.
		--buffer;
		// Is the current character '\xC2' or '\xC3'?
		if(IS_LATIN1(buffer.get())) {	// BUGGED
			// Yep. Remove one extra byte.
			buffer.remove();
		}
		// Go back to the other byte, and remove it.
		++buffer;
		buffer.remove();
		// And move the term back.
		term_left();
		// Print removing the last character.
		printRight(true);
		return;
	} else if(a == "[SUPR]") {
		if(buffer.isLast()) return;

		// Supr is just moving forward and backspace.
		++buffer; term_right();
		if(IS_LATIN1(buffer.get())) ++buffer;
		bk("\b");
		return;
	} else if(a == "[LEFT]") {
		if(buffer.isBeginning()) return;
		term_left();
		--buffer;
		if(!(buffer.isBeginning()) && IS_LATIN1(buffer.get()))
			--buffer;
		return;
	} else if(a == "[RIGHT]") {
		if(buffer.isLast()) return;
		term_right();
		if(IS_LATIN1(buffer.get()))
			++buffer;
		++buffer;
		return;
	} else if(a == "[HOME]") {
		while(!(buffer.isBeginning())) bk("[LEFT]");
		return;
	} else if(a == "[END]") {
		while(!(buffer.isLast())) bk("[RIGHT]");
		return;
	} else if(a == "\n") {
		returnPressed = true;
		while(!(buffer.isLast())) {
			term_right();
			++buffer;
		}
	}

	// Insert the new characters.
	for(auto const& x : a) {
		buffer.put(x);
		if(show) printf("%c", x);
	}

	printRight(false);
}

window<char>& keyboard_getBuffer() { return buffer; }
bool keyboard_returnPressed() { return returnPressed; }

extern "C" void keyboard_handler(void) {
	outb(PIC_IO_PIC1, PIC_EOI);

	uint8_t status = inb(KEYBOARD_STATUS_PORT);

	if(!(status & 0x01)) {
		// Lowest bit is NOT set. The buffer is empty.
		return;
	}

	uint8_t keycode = inb(KEYBOARD_DATA_PORT);

	if(keycode == 0xE0) differentiate = true;
	//printf("%x ", keycode); return;

	const char* handle = 0;
	// Check first some special cases.
	switch(keycode) {
		case 0x38:
			// Altgr down
			if(differentiate) {
				altgr = true;
				handle = (char*)1;
			}
			break;
		case 0xB8:
			// Altgr up
			if(differentiate) {
				altgr = false;
				handle = (char*)1;
			}
			break;
		case 0x2A:
			// Left shift down
			Lshift = true;
			handle = (char*)1;
			break;
		case 0xAA:
			// Left shift up
			Lshift = false;
			break;
		case 0x36:
			// Right shift down
			Rshift = true;
			handle = (char*)1;
			break;
		case 0xB6:
			// Right shift up
			Rshift = false;
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
			accent0 = accent1 = accent2 = accent3 = false;
			bk("\b");
			break;
		case 0x3A:
			// Mayus block
			handle = (char*)1;
			mayb = !mayb;
			break;
		case 0x45:
			// Num block. I'm not interested.
			handle = (char*)1;
			break;
		case 0x1A:
			// `
			if(altgr) break;
			handle = (char*)1;
			if(Lshift || Rshift) {
				// ^
				if(accent0 || accent1 || accent2 || accent3) {
					bk("^");
					accent0 = accent1 = accent2 = accent3 = false;
				} else {
					accent0 = true;
				}
			} else {
				// `
				if(accent0 || accent1 || accent2 || accent3) {
					bk("`");
					accent0 = accent1 = accent2 = accent3 = false;
				} else {
					accent1 = true;
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
					accent0 = accent1 = accent2 = accent3 = false;
				} else {
					accent3 = true;
				}
			} else {
				// ´
				if(accent0 || accent1 || accent2 || accent3) {
					bk("´");
					accent0 = accent1 = accent2 = accent3 = false;
				} else {
					accent2 = true;
				}
			}
			break;
		case 0x4B:
			// <-
			if(!differentiate) break;
			handle = (char*)1;
			bk("[LEFT]");
			break;
		case 0x53:
			// Supr
			if(!differentiate) break;
			handle = (char*)1;
			accent0 = accent1 = accent2 = accent3 = false;
			bk("[SUPR]");
			break;
		case 0x4D:
			// ->
			if(!differentiate) break;
			handle = (char*)1;
			bk("[RIGHT]");
			break;
		case 0x47:
			// Home
			if(!differentiate) break;
			handle = (char*)1;
			bk("[HOME]");
			break;
		case 0x4F:
			// End
			if(!differentiate) break;
			handle = (char*)1;
			bk("[END]");
			break;
		case 0x5B:
			// Left super
			if(!differentiate) break;
			handle = (char*)1;
			break;
		case 0x5C:
			// Right super
			if(!differentiate) break;
			handle = (char*)1;
			break;
	}
	if(handle) {
		differentiate = false;
		return;
	}

	if(keycode >= 128) return;	// Key release.

	if(altgr)
		handle = KB_LAYOUT_ALTGR[keycode];
	else if(Lshift || Rshift || mayb) {
		if(accent0) handle = KB_LAYOUT_SHIFT_AC0[keycode];
		else if(accent1) handle = KB_LAYOUT_SHIFT_AC1[keycode];
		else if(accent2) handle = KB_LAYOUT_SHIFT_AC2[keycode];
		else if(accent3) handle = KB_LAYOUT_SHIFT_AC3[keycode];
		else handle = KB_LAYOUT_SHIFT[keycode];
	} else {
		if(accent0) handle = KB_LAYOUT_AC0[keycode];
		else if(accent1) handle = KB_LAYOUT_AC1[keycode];
		else if(accent2) handle = KB_LAYOUT_AC2[keycode];
		else if(accent3) handle = KB_LAYOUT_AC3[keycode];
		else handle = KB_LAYOUT[keycode];
	}

	if(!handle) {
		if(accent0 || accent1 || accent2 || accent3) {
			if(accent0) {
				bk("^");
			} else if(accent1) {
				bk("`");
			} else if(accent2) {
				bk("´");
			} else if(accent3) {
				bk("¨");
			}
			accent0 = accent1 = accent2 = accent3 = false;
			handle = KB_LAYOUT[keycode];
			if(handle) bk(handle);
		}
		return;
	}

	bk(handle);
	differentiate = false;
	accent0 = accent1 = accent2 = accent3 = false;
}

void keyboard_init() {
	IDT_SET_ENT(IDT[KEYBOARD_IDT_ENTRY], 0, _KERNEL_CODESEGMENT, (uint32_t)IDT_keyboard, 0);

	// Discard already pressed keys.
	inb(KEYBOARD_DATA_PORT);
	inb(KEYBOARD_STATUS_PORT);

	// Set typematic rate and delay.
	outb(KEYBOARD_DATA_PORT, 0xF3);
	while (inb(KEYBOARD_STATUS_PORT) & 2) {}
	outb(KEYBOARD_DATA_PORT, 0x20);
	while(inb(KEYBOARD_DATA_PORT) != 0xFA) {}
}

void keyboard_pause() {
	pic_disable_irq(1);
}
void keyboard_resume(bool show_) {
	// Reset the buffer.
	buffer.clear();
	returnPressed = false;
	show = show_;
	pic_enable_irq(1);
}

void keyboard_setSilentMode() { silentMode = true; }
bool keyboard_getSilentMode() { return silentMode; }
const string& keyboard_getSilentPressed() { return silentPressed; }

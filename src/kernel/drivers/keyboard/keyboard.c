#include <kernel/drivers/keyboard/keyboard.h>
#include <libc/stdio.h>
#include <kernel/asm.h>
#include <kernel/GDT/GDT.h>
#include <kernel/PIC/PIC.h>
#include <kernel/IDT/IDT.h>
#include <kernel/kernel_panic/kernel_panic.h>

void keyboard_handler(void) {
	outb(PIC_IO_PIC1, PIC_EOI);

	uint8_t status = inb(KEYBOARD_STATUS_PORT);

	if(status & 0x01) {
		// Lowest bit is set. The buffer is NOT empty.
		char keycode = inb(KEYBOARD_DATA_PORT);
		if(keycode < 0) return;
		if(keycode == 0x01)
			kernel_panic(0);
		else
			writec(keycode);
	}
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

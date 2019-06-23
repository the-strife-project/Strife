#include <kernel/drivers/keyboard.h>
#include <stdio.h>
#include <asm.h>
#include <GDT.h>
#include <IDT.h>
#include <PIC.h>

void keyboard_handler(void) {
	writec('A');

	outb(PIC_IO_PIC1, PIC_EOI);
}

void keyboard_init(void) {
	IDT_SET_ENT(IDT[KEYBOARD_IDT_ENTRY], 0, _KERNEL_CODESEGMENT, (uint32_t)IDT_keyboard, 0);
	keyboard_resume();
}

void keyboard_pause(void) {
	pic_disable_irq(1);
}
void keyboard_resume(void) {
	pic_enable_irq(1);
}

#include <kernel/drivers/tty.h>
#include <libc/stdio.h>
#include <kernel/GDT.h>
#include <kernel/PIC.h>
#include <kernel/IDT.h>
#include <kernel/drivers/keyboard.h>

void kernel_main(void) {
	terminal_initialize();
	writes("jotadOS NO-VERSION (yet)\n\n");

	writes("Starting GDT...\n");
	gdt_init();

	writes("Starting PIC...\n");
	pic_init();

	writes("Starting IDT...\n");
	idt_init();

	writes("Starting keyboard...\n");
	keyboard_init();
}

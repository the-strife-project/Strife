#include <tty.h>
#include <stdio.h>
#include <GDT.h>
#include <PIC.h>
#include <IDT.h>
#include <kernel/drivers/keyboard.h>

void writes(const char* data) {
	for(size_t i=0; i<strlen(data); i++) writec(data[i]);
}

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

#include <tty.h>
#include <stdio.h>
#include <GDT.h>
#include <IDT.h>

void writes(const char* data) {
	for(size_t i=0; i<strlen(data); i++) writec(data[i]);
}

void kernel_main(void) {
	// Initialize GDT.
	gdt_init();

	// Initialize IDT.
	idt_init();

	terminal_initialize();
	writes("Hello, kernel World!\nAnother line bites the dust.\n");
}

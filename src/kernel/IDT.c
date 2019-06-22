#include <IDT.h>
#include <types.h>

void idt_init(void) {
	struct IDT_entry IDT[256];
	uint32_t irq1_address = (uint32_t)irq1;

	// Remapping the PIC.
	// ICW1: begin initialization.
	write_port(0x20, 0x11);
	write_port(0xA0, 0x11);
	// ICW2: remap address of IDT.
	write_port(0x21, 0x20);
	write_port(0xA1, 0x28);
	// ICW3: setup cascading (master/slaves).
	write_port(0x21, 0x00);
	write_port(0xA1, 0x00);
	// ICW4: environment info. Running in 8086.
	write_port(0x21, 0x01);
	write_port(0xA1, 0x01);
	// Initialization finished.

	// Disable all IRQs at the moment.
	write_port(0x21 , 0xFF);
	write_port(0xA1 , 0xFF);

	IDT[0x21].off_low = irq1_address & 0xFFFF;
	IDT[0x21].selector = 0x08;
	IDT[0x21].zero = 0;
	IDT[0x21].type_attr = 0x8E;
	IDT[0x21].off_high = (irq1_address & 0xFFFF0000) >> (4*4);

	// Fill IDT.
	struct IDT_ptr idtptr;
	idtptr.size = sizeof(IDT);
	idtptr.base = (uint32_t)IDT;

	// Load it.
	load_idt(&idtptr);

	// Enable ONLY the keyboard (IRQ 1).
	write_port(0x21 , 0xFD);
}

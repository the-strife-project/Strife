/*
	This whole file is almost a copy-paste of "PROTURA".
	Check it out, it's listed in the sources.
*/

#include <kernel/asm.h>
#include <kernel/drivers/PIC/PIC.h>

static uint16_t irqmask = 0xFFFF & ~(1<<PIC_IRQ_SLAVE);

void pic_set_mask(void) {
	int eflags = eflags_read();
    cli();

    outb(PIC_IO_PIC1 + 1, irqmask);
    outb(PIC_IO_PIC2 + 1, irqmask >> 8);

	eflags_write(eflags);
}

void pic_enable_irq(int irq) {
	irqmask = irqmask & ~(1 << irq);
	pic_set_mask();
	sti();
}

void pic_disable_irq(int irq) {
	irqmask = irqmask & (1 << irq);
	pic_set_mask();
	sti();
}

// Note: ICW = Initialization command words
void pic_init(void) {
	//pic_set_mask();

	// ICW1: begin initialization.
    outb(PIC_IO_PIC1, 0x11);
    outb(PIC_IO_PIC2, 0x11);

	// ICW2: remap offset address of IDT
    outb(PIC_IO_PIC1 + 1, PIC_IRQ0);
    outb(PIC_IO_PIC2 + 1, PIC_IRQ0 + 8);

	// ICW3: setup cascading
    outb(PIC_IO_PIC1 + 1, 1 << PIC_IRQ_SLAVE);
    outb(PIC_IO_PIC2 + 1, PIC_IRQ_SLAVE);

	// ICW4: environment info
    outb(PIC_IO_PIC1 + 1, 0x01);
    outb(PIC_IO_PIC2 + 1, 0x01);

	pic_set_mask();
}

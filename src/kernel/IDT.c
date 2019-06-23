#include <kernel/IDT.h>
#include <kernel/GDT.h>
#include <common/types.h>
#include <libc/stdio.h>
#include <kernel/PIC.h>
#include <kernel/asm.h>
#include <libc/string.h>	// TODO: remove

void idt_init(void) {
	for(int i=0; i<256; i++) {
		IDT_SET_ENT(IDT[i], 0, _KERNEL_CODESEGMENT, (uint32_t)unhandled_interrupt, 0);
	}

	struct IDT_ptr idtptr;
	idtptr.size = sizeof(IDT)-1;
	idtptr.base = (uint32_t)&IDT;
	load_idt(&idtptr);
}

/* TODO: MOVE THIS SHIT! */
const char LMAO[] = "0123456789ABCDEF";
void printHex(uint32_t xd) {
	writec('0');
	writec('x');

	uint8_t h1 = (uint8_t)(xd >> 4*3);
	uint8_t l1 = (uint8_t)(xd >> 4*2);
	uint8_t h0 = (uint8_t)(xd >> 4);
	uint8_t l0 = (uint8_t)(xd);

	writec(LMAO[h1 / 16]);
	writec(LMAO[h1 % 16]);
	writec(LMAO[l1 / 16]);
	writec(LMAO[l1 % 16]);
	writec(LMAO[h0 / 16]);
	writec(LMAO[h0 % 16]);
	writec(LMAO[l0 / 16]);
	writec(LMAO[l0 % 16]);
}

void unhandled_interrupt_handler(struct irq_frame *iframe) {
	// TODO: modularize "writes"
	char toWrite[] = "Interrupt or exception NOT HANDLED: ";
	for(size_t i=0; i<strlen(toWrite); i++) writec(toWrite[i]);

	printHex((*iframe).intno);
	writec(' ');
	printHex((*iframe).err);
	writec('\n');

	outb(PIC_IO_PIC1, PIC_EOI);
	outb(PIC_IO_PIC2, PIC_EOI);
}

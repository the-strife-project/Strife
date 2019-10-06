#include <kernel/IDT/IDT.h>
#include <kernel/GDT/GDT.h>
#include <common/types.h>
#include <libc/stdio.h>
#include <kernel/drivers/PIC/PIC.h>
#include <kernel/asm.h>
#include <libc/string.h>

void idt_init(void) {
	for(int i=0; i<256; i++) {
		IDT_SET_ENT(IDT[i], 0, _KERNEL_CODESEGMENT, (uint32_t)unhandled_interrupt, 0);
	}

	struct IDT_ptr idtptr;
	idtptr.size = sizeof(IDT)-1;
	idtptr.base = (uint32_t)&IDT;
	load_idt(&idtptr);
}

void unhandled_interrupt_handler(struct irq_frame *iframe) {
	printf("E: 0x%x - 0x%x: %x\n", (*iframe).intno, (*iframe).err, (*iframe).eip);

	// (Bochs breakpoint)
	outw(0x8A00,0x8A00);outw(0x8A00,0x08AE0);

	outb(PIC_IO_PIC1, PIC_EOI);
	outb(PIC_IO_PIC2, PIC_EOI);
}

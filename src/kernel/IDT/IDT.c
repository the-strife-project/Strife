#include <kernel/IDT/IDT.h>
#include <common/types.h>
#include <libc/stdio.h>
#include <kernel/drivers/PIC/PIC.h>
#include <kernel/asm.h>

// This is a bitmap to mark those interrupts that should be ignored.
// That is, if they have the default ISR, do NOT print a debug output.
uint32_t ISR_ignore[8] = {0};

void idt_init(void) {
	for(int i=0; i<256; i++) {
		IDT_SET_ENT(IDT[i], 0, _KERNEL_CODESEGMENT, default_ISR_array[i], 0);
	}

	struct IDT_ptr idtptr;
	idtptr.size = sizeof(IDT)-1;
	idtptr.base = (uint32_t)&IDT;
	load_idt(&idtptr);
}

void default_interrupt_handler(uint32_t intno) {
	// I don't care about these.
	if(!(ISR_ignore[intno / 32] & (1 << (intno % 32)))) {
		printf("[[[ DISR: 0x%x ]]]\n", intno);
		outw(0x8A00,0x8A00);outw(0x8A00,0x08AE0);	// Bochs breakpoint
	}

	outb(PIC_IO_PIC1, PIC_EOI);
	outb(PIC_IO_PIC2, PIC_EOI);
}

void ISR_ignoreUP(uint8_t intno) { ISR_ignore[intno / 32] |= 1 << (intno % 32); }
void ISR_ignoreDOWN(uint8_t intno) { ISR_ignore[intno / 32] &= ~(1 << (intno % 32)); }

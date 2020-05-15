#include <kernel/IDT/IDT.hpp>
#include <common/types.hpp>
#include <kernel/klibc/stdio>
#include <kernel/drivers/PIC/PIC.hpp>
#include <kernel/asm.hpp>

struct IDT_entry IDT[256];

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

extern "C" void default_interrupt_handler(uint32_t intno) {
	// I don't care about these.
	if(!(ISR_ignore[intno / 32] & (1 << (intno % 32)))) {
		switch(intno) {
			// These are taken from the Intel's Software Developer's Manual.
			case 0:
				printf("\n[[[ DIVIDE ERROR EXCEPTION ]]]\n");
				break;
			case 5:
				printf("\n[[[ BOUND RANGE EXCEEDED EXCEPTION ]]]\n");
				break;
			case 6:
				printf("\n[[[ INVALID OPCODE EXCEPTION ]]]\n");
				while(true) {}
				break;
			case 7:
				printf("\n[[[ DEVICE NOT AVAILABLE EXCEPTION ]]]\n");
				break;
			case 8:
				printf("\n[[[ DOUBLE FAULT EXCEPTION ]]]\n");
				break;
			case 10:
				printf("\n[[[ INVALID TSS EXCEPTION ]]]\n");
				break;
			case 11:
				printf("\n[[[ SEGMENT NOT PRESENT ]]]\n");
				break;
			case 12:
				printf("\n[[[ STACK FAULT EXCEPTION ]]]\n");
				break;
			case 13:
				printf("\n[[[ GENERAL PROTECTION EXCEPTION ]]]\n");
				while(true) {}
				break;
			case 14:
				printf("\n[[[ PAGE FAULT EXCEPTION ]]]\n");
				break;
			case 16:
				printf("\n[[[ FPU FLOATING-POINT ERROR ]]]\n");
				break;
			case 17:
				printf("\n[[[ ALIGNMENT CHECK EXCEPTION ]]]\n");
				break;
			case 32:
				/*
					Interrupt 0x20 fired by ATAPI_PIO. Anything else fires it?
					I don't know. By now, ignore it...

					TODO: remove this special case once I implement DMA.
				*/
				break;
			default:
				printf("\n[[[ DISR: 0x%x ]]]\n", intno);
				while(true) {}
		}
		outw(0x8A00,0x8A00);outw(0x8A00,0x08AE0);	// Bochs breakpoint
	}

	outb(PIC_IO_PIC1, PIC_EOI);
	outb(PIC_IO_PIC2, PIC_EOI);
}

void ISR_ignoreUP(uint8_t intno) { ISR_ignore[intno / 32] |= 1 << (intno % 32); }
void ISR_ignoreDOWN(uint8_t intno) { ISR_ignore[intno / 32] &= ~(1 << (intno % 32)); }

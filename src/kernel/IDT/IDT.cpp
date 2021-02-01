#include <kernel/IDT/IDT.hpp>
#include <common/types.hpp>
#include <kernel/klibc/stdio>
#include <kernel/drivers/PIC/PIC.hpp>
#include <kernel/paging/paging.hpp>
#include <kernel/drivers/clock/clock.hpp>

IDT_entry IDT[256];

/*
	ISR delegates.
	topesp (Top of ESP register) contains at the top PushadRegs, and then
	iretValues.
*/
ISR_delegate_t delegates[256];

void idt_init(void) {
	for(int i=0; i<256; i++) {
		IDT_SET_ENT(IDT[i], 0, _KERNEL_CODESEGMENT, default_ISR_array[i], 0);
		delegates[i] = (ISR_delegate_t)0;
	}

	struct IDT_ptr idtptr;
	idtptr.size = sizeof(IDT)-1;
	idtptr.base = (uint32_t)&IDT;
	load_idt(&idtptr);
}

extern "C" void default_interrupt_handler(uint32_t intno, uint32_t topesp) {
	if(delegates[intno]) {
		uint32_t cr3 = getCR3();
		kernelPaging.use();

		(*(delegates[intno]))(topesp);

		// If returned, set previous page directory and let's call it a day.
		Paging((uint32_t*)cr3).use();
		return;
	}

	uint32_t error = ISR_get_error(topesp);

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
			break;
		case 14:
			printf("\n[[[ PAGE FAULT EXCEPTION ]]]\n");
			printf("\tPage: %x   Error: ", getCR2());
			if(error & 0b1)
				printf("(Page protection) ");
			else
				printf("(Non present) ");
			if(error & 0b10)
				printf("(Write) ");
			else
				printf("(Read) ");
			if(error & 0b100)
				printf("(User) ");
			if(error & 0b1000)
				printf("(Reserved) ");
			if(error & 0b10000)
				printf("(IF)");
			clock_disable();
			while(true);
		case 16:
			printf("\n[[[ FPU FLOATING-POINT ERROR ]]]\n");
			break;
		case 17:
			printf("\n[[[ ALIGNMENT CHECK EXCEPTION ]]]\n");
			break;
		default:
			printf("\n[[[ DISR: 0x%x ]]]\n", intno);
			break;
	}

	while(true) {}

	outb(PIC_IO_PIC1, PIC_EOI);
	outb(PIC_IO_PIC2, PIC_EOI);
}

void IDT_setPL(uint32_t intno, uint8_t pl) {
	IDT[intno].dpl = pl;
}

void ISR_delegate(uint32_t intno, ISR_delegate_t delegate) {
	delegates[intno] = delegate;
}

PushadRegs* ISR_get_PushadRegs(uint32_t esp) {
	return (PushadRegs*)(esp);
}

iretValues* ISR_get_iretValues(uint32_t esp) {
	return (iretValues*)(esp + sizeof(PushadRegs));
}

uint32_t ISR_get_error(uint32_t esp) {
	return *(uint32_t*)(esp + sizeof(PushadRegs) + sizeof(iretValues));
}

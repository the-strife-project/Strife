#include <kernel/V86/V86.h>
#include <kernel/paging/paging.h>

#include <kernel/asm.h>
void V86(uint8_t intnum, struct regs16_t *regs) {
	// Disable paging.
	paging_disable();

	// Run the interrupt.
	int32(intnum, regs);

	// Enable paging again.
	paging_enable();
}

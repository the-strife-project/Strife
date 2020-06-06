#include <kernel/kernel_panic/kernel_panic.hpp>
#include <common/types.hpp>
#include <kernel/drivers/term/term.hpp>
#include <kernel/klibc/stdio>
#include <kernel/asm.hpp>

static const char* kp_messages[] = {
	[KP_EXAMPLE] = "Kernel panic example.",
	[KP_OUTOFMEM] = "Out of memory :(",
	[KP_MULTIBOOT] = "Invalid multiboot signature!",
	[KP_VBE2] = "Your BIOS does not support VBE2.",
	[KP_GRAPHICS] = "Your graphics card does not support 800x600x32.",
	[KP_PCI] = "PCI error. See the message above.",
	[KP_DMA_BUSMASTER] = "ATA bus master register is not an I/O port.",
	[KP_NO_LBA] = "ATA drive does not support LBA addressing.",
	[KP_CD_NOT_FOUND] = "IDE drive appears to be a CD, but no such boot CD was found.",
	[KP_HDD_NOT_FOUND] = "IDE drive appears to be an HDD, but none was found with matching UUID."
};

void panic(uint32_t id) {
	printf("\n");

	// Change colors.
	term_setBGC(0x4);
	term_setFGC(0xF);

	// Show the error.
	printf("\nKERNEL PANIC!\n%s\n\nPlease, reboot.", kp_messages[id]);

	// Halt.
	cli();
	while(1) {}
}

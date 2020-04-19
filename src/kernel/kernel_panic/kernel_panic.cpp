#include <kernel/kernel_panic/kernel_panic.hpp>
#include <common/types.hpp>
#include <kernel/drivers/term/term.hpp>
#include <kernel/klibc/stdio>
#include <kernel/asm.hpp>

static const char* kp_messages[] = {
	[0] = "Kernel panic example.",
	[1] = "Out of memory :(",
	[2] = "Invalid multiboot signature!",
	[3] = "Your BIOS does not support VBE2.",
	[4] = "Your graphics card does not support 800x600x32."
};

void kernel_panic(uint32_t id) {
	// Change colors.
	term_setBGC(0x4);
	term_setFGC(0xF);

	// Show the error.
	printf("\nKERNEL PANIC!\n%s\n\nPlease, reboot.", kp_messages[id]);

	// Halt.
	cli();
	while(1) {}
}

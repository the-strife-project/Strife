#include <kernel/kernel_panic/kernel_panic.h>
#include <common/types.h>
#include <kernel/drivers/term/term.h>
#include <libc/stdio.h>
#include <kernel/asm.h>

static const char* kp_messages[] = {
	[0] = "Kernel panic example.",
	[1] = "Out of memory :(",
	[2] = "Invalid multiboot signature!",
	[3] = "Your BIOS does not support VBE2.",
	[4] = "Your graphics card does not support 800x600x32."
};

void kernel_panic(uint32_t id) {
	// Fill screen with red.
	if(!term_getCurrentMode()) {
		// Text.
		uint8_t bgcolor = 12 << 4;
		bgcolor |= 1;
		term_setBGC(bgcolor);
	} else {
		// Graphics.
		term_setBGC(0x00FF0000);
		term_setFGC(0x00FFFFFF);
	}


	// Show the error.
	term_clear();
	printf("\nKERNEL PANIC!\n%s\n\nPlease, reboot.", kp_messages[id]);

	// Don't to anything.
	cli();
	while(1) {}
}

#include <kernel/kernel_panic/kernel_panic.h>
#include <common/types.h>
#include <common/colors.h>
#include <kernel/drivers/TTY/TTY.h>
#include <libc/stdio.h>
#include <kernel/asm.h>

static const char* kp_messages[] = {
	[0] = "Kernel panic example.",
	[1] = "Out of memory :(",
	[2] = "Invalid multiboot signature!"
};

void kernel_panic(uint32_t id) {
	// Fill screen with red.
	uint8_t bgcolor = VGA_COLOR_LIGHT_RED << 4;
	bgcolor |= VGA_COLOR_BLUE;
	terminal_fill(bgcolor);

	// Show the error.
	terminal_goStart();
	printf("\nKERNEL PANIC!\n%s\n\nPlease, reboot.", kp_messages[id]);

	// Don't to anything.
	cli();
	while(1) {}
}

#include <kernel/drivers/term/term.h>
#include <libc/stdio.h>
#include <libc/stdlib.h>
#include <kernel/GDT/GDT.h>
#include <kernel/paging/paging.h>
#include <kernel/drivers/PIC/PIC.h>
#include <kernel/IDT/IDT.h>
#include <kernel/drivers/keyboard/keyboard.h>
#include <kernel/drivers/clock/clock.h>
#include <kernel/kernel_panic/kernel_panic.h>
#include <kernel/asm.h>
#include <kernel/PCI/PCI.h>
#include <kernel/memutils/memutils.h>
#include <kernel/drivers/VESA/VESA.h>
#include <kernel/splash.h>
#include <kernel/install/install.h>

#define bochs_breakpoint() outw(0x8A00,0x8A00);outw(0x8A00,0x08AE0);

void kernel_main(void) {
	memutils_init();
	term_init();

	printf("Setting GDT...\n");
	gdt_init();

	printf("Beginning paging...\n");
	paging_init();
	paging_enable();

	printf("Remapping PIC...\n");
	pic_init();

	printf("Setting IDT...\n");
	idt_init();

	printf("Loading drivers...\n");
	keyboard_init();
	clock_init();
	clock_start();

	printf("Going graphics. See you in a bit...\n");
	VESA_init(800, 600, 32);
	term_goGraphics(800, 600);
	term_setFGC(0x0000FF00);

	showSplash("jotadOS", 4, (800-(4*8*7))/2, 32);

	// Check where we're booting from.
	uint8_t bootDriveID = (uint8_t)(*((uint8_t*)0x9000));
	if(bootDriveID == 0xE0) {
		// It's the CD. Run the installation program.
		/*
			This REALLY should not be part of the kernel.
			Might change it at some point, I don't care tbh.
		*/
		//install();
		// That should not return.
	}

	printf("Kernel size: %dKiB\n", (int)ASM_KERNEL_END>>10);
	printf("%dKiB of RAM available.\n", getFreeMemory());
	printf("\nGo ahead, type something\n");
	showCursor();

	while(1) {
		printf("> ");
		char* r = readLine();
		printf("< (%d) %s\n", strlen(r), r);
		jfree(r);
	}
}

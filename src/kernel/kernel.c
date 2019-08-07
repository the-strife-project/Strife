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
#include <common/elf.h>
#include <kernel/PCI/PCI.h>
#include <kernel/memutils/memutils.h>
#include <kernel/drivers/VESA/VESA.h>
#include <kernel/splash.h>
#include <kernel/drivers/ATA_PIO/ATA_PIO.h>

#define bochs_breakpoint() outw(0x8A00,0x8A00);outw(0x8A00,0x08AE0);

void kernel_main(void) {
	memutils_init();
	term_init();

	printf("Setting GDT...\n");
	gdt_init();

	printf("Beginning paging...\n");
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

	printf("%dK of RAM available.\n", getFreeMemory());

	// ATA TEST
	/*struct ATA_INTERFACE* test = newATA(1, 0x1F0);
	if(ATA_identify(test) != 0) {
		printf("This hard disk cannot be accessed.\n");
		while(1) {}
	}
	if(ATA_write28(test, 0, (uint8_t*)"Hello! This is a test!\n\0") != 0) {
		printf("Something went utterly wrong.\n");
		while(1) {}
	}
	char* firstSector = (char*)ATA_read28(test, 0);
	printf("Read: %s", firstSector);
	jfree(firstSector);*/

	printf("\nGo ahead, type something\n");
	showCursor();

	while(1) {
		printf("> ");
		char* r = readLine();
		printf("< (%d) %s\n", strlen(r), r);
		jfree(r);
	}
}

#include <kernel/drivers/term/term.h>
#include <klibc/stdio.h>
#include <klibc/stdlib.h>
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
#include <kernel/install/install.h>
#include <kernel/syscalls/syscalls.h>
#include <kernel/TSS/TSS.h>
#include <kernel/usermode/usermode.h>
#include <kernel/drivers/storage/FS/JOTAFS/JOTAFS.h>

#ifdef STL_TEST
#include <klibc/STL/test.h>
#endif

#define bochs_breakpoint() outw(0x8A00,0x8A00);outw(0x8A00,0x8AE0);

#include <klibc/STL/vector>
extern "C" void kernel_main(void) {
	memutils_init();
	term_setFGC(0xA);
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
	//clock_init();
	//clock_start();

	printf("\n");
	term_setFGC(0x8);
	term_setBGC(0xB);
	printf("\n                                    jotadOS                                     \n");
	term_setFGC(0xA);
	term_setBGC(0x0);
	printf("\n");

	// Test STL.
	#ifdef STL_TEST
	STL_test();
	while(true) {}
	#endif

	// Check where we're booting from.
	uint8_t bootDriveID = (uint8_t)(*((uint8_t*)0x9000));
	if(bootDriveID == 0xE0) {
		// It's the CD. Run the installation program.
		/*
			TODO: don't run the installation if we boot from CD.
			Instead, improve the ATAPI driver and make this usable.
		*/
		/*
			TODO: install() should REALLY not be part of the kernel.
			Might change it at some point, I don't care tbh.
		*/
		install();
		// That should not return.
	}

	// Enable syscalls.
	syscalls_init();

	// Load the TSS.
	TSS_flush();

	// Initialize the file system.
	ATA primarymaster(1, 0x1F0);
	JOTAFS jotafs(primarymaster);

	// Run the MSS.
	uint32_t mss = paging_allocPages(1);
	paging_setUser(mss, 1);
	jotafs.readWholeFile(3, (uint8_t*)mss);
	jump_usermode(mss);

	printf("\n[[[ MSS RETURNED?!?!?! ]]]");
	while(1) {}
}
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

#define bochs_breakpoint() outw(0x8A00,0x8A00);outw(0x8A00,0x08AE0);

void kernel_main(void) {
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

	// Check where we're booting from.
	uint8_t bootDriveID = (uint8_t)(*((uint8_t*)0x9000));
	if(bootDriveID == 0xE0) {
		// It's the CD. Run the installation program.
		/*
			This should NOT happen.
			TODO: improve the ATAPI driver and use something like ramfs
			so that jotadOS can run from a CD in a clean way.
		*/
		/*
			This REALLY should not be part of the kernel.
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
	struct ATA_INTERFACE* primarymaster = newATA(1, 0x1F0);
	JOTAFS_init(primarymaster);

	// Run the MSS (Main Shell System).
	uint32_t mss = paging_allocPages(2);
	paging_setUser(mss, 2);
	JOTAFS_readwholefile(4, (uint8_t*)(mss+4096));
	jump_usermode(mss+4096);

	printf("\n[[[ MSS RETURNED?!?!?! ]]]");
	while(1) {}
}

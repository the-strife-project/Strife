#include <kernel/drivers/term/term.hpp>
#include <klibc/stdio>
#include <klibc/stdlib.hpp>
#include <kernel/GDT/GDT.hpp>
#include <kernel/paging/paging.hpp>
#include <kernel/drivers/PIC/PIC.hpp>
#include <kernel/IDT/IDT.hpp>
#include <kernel/drivers/keyboard/keyboard.hpp>
#include <kernel/drivers/clock/clock.hpp>
#include <kernel/kernel_panic/kernel_panic.hpp>
#include <kernel/asm.hpp>
#include <kernel/PCI/PCI.hpp>
#include <kernel/memutils/memutils.hpp>
#include <kernel/install/install.hpp>
#include <kernel/syscalls/syscalls.hpp>
#include <kernel/TSS/TSS.hpp>
#include <kernel/usermode/usermode.hpp>
#include <kernel/drivers/storage/FS/JOTAFS/JOTAFS.hpp>
#include <kernel/drivers/storage/FS/ISO9660/ISO9660.hpp>
#include <kernel/drivers/term/font/font.hpp>

void printSplash() {
	printf("\n");
	term_setFGC(0x8);
	term_setBGC(0xB);
	printf("\n                                    jotadOS                                     \n");
	term_setFGC(0xA);
	term_setBGC(0x0);
	printf("\n");
}

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

	// Check where we're booting from.
	uint8_t bootDriveID = (uint8_t)(*((uint8_t*)0x9000));
	if(bootDriveID == 0xE0) {
		// It's the CD. Run the installation program.

		// Load font.
		printf("Loading font...\n");
		list<string> font_p(string("FONT/LAT1/LAT1.RAW").split('/'));
		ISO9660_entity* font_e = ISO9660_get(font_p);
		if(font_e) {
			uint8_t* font = ISO9660_read(font_e);
			setFont(font);
			jfree(font);
			loadFontToVGA();
		}
		jfree(font_e);

		printSplash();

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

	// Initialize the file system.
	ATA primarymaster(1, 0x1F0);
	JOTAFS jotafs(primarymaster);

	// Load font.
	printf("Loading font...\n");
	uint8_t* font = jotafs.readWholeFile(jotafs.find("/sys/fonts/lat1-16/lat1-16.font"));
	setFont(font);
	jfree(font);
	loadFontToVGA();

	printSplash();

	// Enable syscalls.
	syscalls_init();

	// Load the TSS.
	TSS_flush();

	// Where's MSS?
	uint32_t mss_inode_n = jotafs.find("/bin/core/mss");

	// Load it and run it.
	uint32_t mss = paging_allocPages(1);
	paging_setUser(mss, 1);
	/*
		This number 4 is temporally hardcoded, created during install.
		This will be, in the future, read from the directory tree.
	*/
	jotafs.readWholeFile(mss_inode_n, (uint8_t*)mss);
	jump_usermode(mss);

	printf("\n[[[ MSS RETURNED?!?!?! ]]]");
	while(1) {}
}

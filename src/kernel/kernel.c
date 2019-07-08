#include <kernel/drivers/term/term.h>
#include <libc/stdio.h>
#include <libc/stdlib.h>
#include <kernel/GDT/GDT.h>
#include <kernel/paging/paging.h>
#include <kernel/drivers/PIC/PIC.h>
#include <kernel/IDT/IDT.h>
#include <kernel/drivers/keyboard/keyboard.h>
#include <kernel/drivers/clock/clock.h>
#include <boot.h>
#include <kernel/kernel_panic/kernel_panic.h>
#include <kernel/asm.h>
#include <common/elf.h>
#include <kernel/PCI/PCI.h>
#include <kernel/memutils/memutils.h>
#include <kernel/drivers/VESA/VESA.h>
#include <kernel/splash.h>

#define bochs_breakpoint() outw(0x8A00,0x8A00);outw(0x8A00,0x08AE0);

void kernel_main(uint32_t multiboot_magic, struct multiboot_info* mbinfo) {
	memutils_init(mbinfo);
	term_init();

	if (multiboot_magic != 0x2BADB002) {
		kernel_panic(2);
	}

	printf("Setting GDT...\n");
	gdt_init();

	printf("Beginning paging...\n");
	paging_enable();

	// Protect modules.
	multiboot_module_t* mods = (multiboot_module_t*)mbinfo->mods_addr;
    for (uint32_t i = 0; i < mbinfo->mods_count; i++) {
        paging_setPresent(mods[i].mod_start, ((mods[i].mod_end - mods[i].mod_start) / 4096) + 1);
	}

	// Protect kernel symbols.
	paging_setPresent(mbinfo->u.elf_sec.addr, paging_sizeToPages(mbinfo->u.elf_sec.size * mbinfo->u.elf_sec.num));
    struct ELFSectionHeader_t* kern_sections = (struct ELFSectionHeader_t*)mbinfo->u.elf_sec.addr;
    for (uint32_t i = 0; i < mbinfo->u.elf_sec.num; i++) {
        paging_setPresent(kern_sections[i].addr, paging_sizeToPages(kern_sections[i].size));
	}

	printf("Remapping PIC...\n");
	pic_init();

	printf("Setting IDT...\n");
	idt_init();

	printf("Loading drivers...\n");
	keyboard_init();
	clock_init();

	printf("Going graphics. See you in a bit...\n");
	VESA_init(800, 600, 32);
	term_goGraphics(800, 600);
	term_setFGC(0x0000FF00);

	showSplash("jotadOS", 4, (800-(4*8*7))/2, 32);

	printf("%dK of RAM available.\n", getFreeMemory());
	printf("\nGo ahead, type something\n");

	while(1) {
		printf("> ");
		char* r = readLine();
		printf("< (%d) %s\n", strlen(r), r);
	}
}

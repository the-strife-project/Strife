#include <kernel/drivers/term/term.h>
#include <libc/stdio.h>
#include <libc/stdlib.h>
#include <kernel/GDT/GDT.h>
#include <kernel/paging/paging.h>
#include <kernel/PIC/PIC.h>
#include <kernel/IDT/IDT.h>
#include <kernel/drivers/keyboard/keyboard.h>
#include <boot.h>
#include <kernel/kernel_panic/kernel_panic.h>
#include <kernel/asm.h>
#include <common/elf.h>
#include <kernel/PCI/PCI.h>
#include <kernel/drivers/VGA/VGA.h>
#include <kernel/memutils/memutils.h>
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

	printf("Going VGA. See you in a bit...\n");
	VGA_setMode(VGA_WIDTH, VGA_HEIGHT, VGA_COLORDEPTH);
	term_goVGA();
	term_clear();

	printf("jotadOS");

	/*printf("Starting keyboard...\n");
	keyboard_init();

	printf("\nAll set. %dK of RAM available.\n", getFreeMemory());
	printf("\nType something. Press ESC for kernel panic simulation.\n");*/

	while(1) {}
}

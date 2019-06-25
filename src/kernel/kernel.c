#include <kernel/drivers/tty.h>
#include <libc/stdio.h>
#include <libc/stdlib.h>
#include <kernel/GDT.h>
#include <kernel/paging.h>
#include <kernel/PIC.h>
#include <kernel/IDT.h>
#include <kernel/drivers/keyboard.h>
#include <boot.h>
#include <kernel/kernel_panic.h>
#include <kernel/asm.h>
#include <common/elf.h>
#define bochs_breakpoint() outw(0x8A00,0x8A00);outw(0x8A00,0x08AE0);

void kernel_main(uint32_t multiboot_magic, struct multiboot_info* mbinfo) {

	terminal_initialize();
	printf("jotadOS\n\n");

	printf("Setting GDT...\n");
	gdt_init();

	printf("Beginning paging...\n");
	paging_enable();
	bochs_breakpoint();

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

	printf("Starting keyboard...\n");
	keyboard_init();

	uint32_t freeRam = mbinfo->mem_upper;
	freeRam -= paging_getUsedPages()*4;
	printf("\nAll set. %dK of RAM available.\n", freeRam);
	printf("\nType something. Press ESC for kernel panic simulation.\n");

	while(1) {}
}

#include <kernel/drivers/tty.h>
#include <libc/stdio.h>
#include <kernel/GDT.h>
#include <kernel/paging.h>
#include <kernel/PIC.h>
#include <kernel/IDT.h>
#include <kernel/drivers/keyboard.h>
#include <boot.h>
#include <kernel/kernel_panic.h>

void kernel_main(uint32_t multiboot_magic, struct MultibootInfo_t* multiboot_info) {
	terminal_initialize();
	writes("jotadOS NO-VERSION (yet)\n\n");

	if(multiboot_magic != 0x2BADB002) {
		kernel_panic(2);
	}

	writes("Setting GDT...\n");
	gdt_init();

	writes("Beginning paging...\n");
	paging_enable(multiboot_info->mem_upper / 4);

	writes("Testing memory: ");
	uint8_t* free = (uint8_t*)paging_allocPages(1);
	paging_setAbsent((uint32_t)free, 1);
	uint8_t* other = (uint8_t*)paging_allocPages(1);
	if(free == other) {
		writes("OK\n");
	}
	paging_setAbsent((uint32_t)other, 1);

	writes("Remapping PIC...\n");
	pic_init();

	writes("Setting IDT...\n");
	idt_init();

	writes("Starting keyboard...\n");
	keyboard_init();

	writes("\nAll set. (TODO) KiB of RAM available.\n");
	writes("\nType something. Press ESC for kernel panic simulation.\n");

	while(1) {}
}

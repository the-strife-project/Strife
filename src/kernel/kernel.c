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

void kernel_main(uint32_t multiboot_magic, struct MultibootInfo_t* multiboot_info) {
	// Bochs breakpoint
	outw(0x8A00,0x8A00);
	outw(0x8A00,0x08AE0);

	terminal_initialize();
	printf("jotadOS\n\n");

	if(multiboot_magic != 0x2BADB002) {
		kernel_panic(2);
	}

	printf("Setting GDT...\n");
	gdt_init();

	printf("Beginning paging...\n");
	paging_enable(multiboot_info->mem_upper);

	printf("Remapping PIC...\n");
	pic_init();

	printf("Setting IDT...\n");
	idt_init();

	printf("Starting keyboard...\n");
	keyboard_init();

	int freeMemory = multiboot_info->mem_upper;
	freeMemory -= ((uint32_t)ASM_KERNEL_END/1024)+4;
	freeMemory -= 4096;
	printf("\nAll set. %dKiB of RAM available.\n", freeMemory);
	printf("\nType something. Press ESC for kernel panic simulation.\n");

	while(1) {}
}

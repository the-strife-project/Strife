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

void kernel_main(uint32_t multiboot_magic, struct MultibootInfo_t* multiboot_info) {
	terminal_initialize();
	printf("jotadOS\n\n");

	if(multiboot_magic != 0x2BADB002) {
		kernel_panic(2);
	}

	printf("Hola! %s%d %c\n", "TESTO", 23, 'j');

	printf("Setting GDT...\n");
	gdt_init();

	printf("Beginning paging...\n");
	paging_enable(multiboot_info->mem_upper / 4);

	printf("Remapping PIC...\n");
	pic_init();

	printf("Setting IDT...\n");
	idt_init();

	printf("Starting keyboard...\n");
	keyboard_init();

	printf("\nAll set. (TODO) KiB of RAM available.\n");
	printf("\nType something. Press ESC for kernel panic simulation.\n");

	while(1) {}
}

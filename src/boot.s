.set ALIGN,    1<<0		/* Align loaded modules on page boundaries */
.set MEMINFO,  1<<1		/* Provide memory map */
.set FLAGS,    ALIGN | MEMINFO
.set MAGIC,    0x1BADB002
.set CHECKSUM, -(MAGIC + FLAGS)

/* Declare a multiboot header that marks the program as a kernel. */
.section .multiboot
.align 4
.long MAGIC
.long FLAGS
.long CHECKSUM

/* 16K stack */
.section .bss
.align 16
stack_bottom:
.skip 16384
stack_top:


.section .text
.global _start
.type _start, @function
_start:
	mov $stack_top, %esp

	/* The GDT should be loaded here. Paging should be enabled here. */

	call kernel_main

	/* Enter an infinite loop */
1:	hlt
	jmp 1b


.size _start, . - _start

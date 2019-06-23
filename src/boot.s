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
.global STACK_BOTTOM
.global STACK_TOP
STACK_BOTTOM:
.skip 16384	/* 16 KiB */
STACK_TOP:


.section .text
.global _start
.type _start, @function
_start:
	mov $STACK_TOP, %esp
	push %ebx
	push %eax
	call kernel_main
	cli

	/* Enter an infinite loop */
1:	hlt
	jmp 1b

.global ASM_KERNEL_END
ASM_KERNEL_END:
	/* Kernel size detection */

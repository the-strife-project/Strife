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

/* FPU Config */
VAL_037F:
	.hword 0x037F
VAL_037E:
	.hword 0x037E
VAL_037A:
	.hword 0x037A

.global _start
.type _start, @function
_start:
	mov $STACK_TOP, %esp
	push %ebx
	push %eax

	/* Configure FPU */
	cli
	mov %cr0, %eax
	or $0b00110010, %eax
	and $0xFFFFFFFB, %eax
	mov %eax, %cr0
	fldcw VAL_037F
	fldcw VAL_037E
	fldcw VAL_037A
	fninit

	call kernel_main
	cli

1:	hlt
	jmp 1b

.section .sizedetect
.global ASM_KERNEL_END
ASM_KERNEL_END:
	/* Kernel size detection */

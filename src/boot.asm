%define ALIGN    1 ;(1<<0) ; Align loaded modules on page boundaries
%define MEMINFO  2 ;(1<<1) ; Provide memory map
%define FLAGS    3 ;(ALIGN | MEMINFO)
%define MAGIC    0x1BADB002
%define CHECKSUM -(MAGIC + FLAGS)

; Declare a multiboot header that marks the program as a kernel.
section .multiboot
align 4
dd MAGIC
dd FLAGS
dd CHECKSUM

; Set stack
section .bss
align 16
global STACK_BOTTOM
global STACK_TOP

STACK_BOTTOM:
resb 16384
STACK_TOP:


section .text

; FPU Config
VAL_037F:
	dw 0x037F
VAL_037E:
	dw 0x037E
VAL_037A:
	dw 0x037A

global _start
extern kernel_main
_start:
	mov esp, STACK_TOP
	push ebx
	push eax

	; Configure FPU
	;cli
	;mov eax, cr0
	;or eax, 0b00110010
	;and eax, 0xFFFFFFFB
	;mov cr0, eax
	;fldcw [VAL_037F]
	;fldcw [VAL_037E]
	;fldcw [VAL_037A]
	;fninit

	call kernel_main
	cli

hlt_L:
	hlt
	jmp hlt_L

section .sizedetect
global ASM_KERNEL_END
ASM_KERNEL_END:
	; Kernel size detection

; Multiboot? Deez nuts! Ha! Got himmm!
; It's totally not a multiboot header.
; But I need somewhere to locate '_start'.
section .multiboot
dd _start

; Set stack
section .bss
align 16
global STACK_BOTTOM
global STACK_TOP

STACK_BOTTOM:
resb 0x4000
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

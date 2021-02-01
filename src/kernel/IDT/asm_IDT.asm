BITS 32

%define KERNEL_DATASEGMENT 0x10
%define INSTRUCTION_FLAG_MASK 0x0200

global default_ISR
extern default_interrupt_handler

; This is a macro for handling all interrupts by default.
%assign i 0
%rep 256
default_ISR_ %+ i:
	;pop edx	; Error code. TODO: some produce it and some don't.
	pushad
	mov ebx, esp

	push ds
    push es
    push fs
	push gs

	; Load kernel segments.
	mov ax, KERNEL_DATASEGMENT
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax

	; Allow interrupts.
	pushfd
	pop eax
	or eax, INSTRUCTION_FLAG_MASK
	push eax
	popfd

	mov eax, cr2
	push eax
	push ebx
	push i
	call default_interrupt_handler
	add esp, 4*3

	pop gs
    pop fs
    pop es
	pop ds

	popad
	; Should push here error code if needed (or not?)
	iret
%assign i i+1
%endrep


; Now let's create an array of addresses to each default ISR
global default_ISR_array_contents
default_ISR_array_contents:
	%assign i 0
	%rep 256
	dd default_ISR_ %+ i
	%assign i i+1
	%endrep

global default_ISR_array
default_ISR_array:
	dd default_ISR_array_contents


global load_idt
load_idt:
	mov eax, [esp+4]
	lidt [eax]
	ret

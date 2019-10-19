global default_ISR
extern default_interrupt_handler

; This is a macro for handling all interrupts by default.
%assign i 0
%rep 256
default_ISR_ %+ i:
	push ds
    push es
    push fs
	push gs
	pusha

	mov ax, 0x10
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax

	push i
	call default_interrupt_handler
	add esp, 4

	popa
	pop gs
    pop fs
    pop es
	pop ds
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

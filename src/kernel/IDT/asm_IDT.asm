global unhandled_interrupt
global load_idt
extern unhandled_interrupt_handler

unhandled_interrupt:
	push ds
    push es
    push fs
	pusha

	push esp
	call unhandled_interrupt_handler
	add esp, 4

	popa
    pop fs
    pop es
	pop ds
	iret

load_idt:
	mov eax, [esp+4]
	lidt [eax]
	ret

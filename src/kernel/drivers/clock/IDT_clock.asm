global IDT_clock
extern clock_handler

IDT_clock:
	push ds
    push es
    push fs
	pusha

	call clock_handler

	popa
    pop fs
    pop es
	pop ds
	iret

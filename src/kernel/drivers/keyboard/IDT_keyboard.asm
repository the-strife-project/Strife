global IDT_keyboard
extern keyboard_handler

IDT_keyboard:
	push ds
    push es
    push fs
	pusha

	call keyboard_handler

	popa
    pop fs
    pop es
	pop ds
	iret

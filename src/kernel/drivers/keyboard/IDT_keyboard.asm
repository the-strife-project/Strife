global IDT_keyboard
extern keyboard_handler

IDT_keyboard:
	pusha

	call keyboard_handler

	popa
	iret

global IDT_IDE
extern ide_irq_handler

IDT_IDE:
	push ds
    push es
    push fs
	pusha

	call ide_irq_handler

	popa
    pop fs
    pop es
	pop ds
	iret

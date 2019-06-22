.global irq1

.global load_idt
.global read_port
.global write_port

irq1:
	push %ds
	push %es
	push %fs
	push %gs
	pusha

	call keyboard_handler

	popa
	pop %gs
	pop %fs
	pop %es
	pop %ds
	iret

load_idt:
	mov 0x4(%esp), %edx
	lidt (%edx)
	sti	/* Turn interrupts on */
	ret

read_port:
	mov 0x4(%esp), %edx
	in %dx, %al
	ret

write_port:
	mov 0x4(%esp), %edx
	mov 0x8(%esp), %al
	out %al, %dx
	ret

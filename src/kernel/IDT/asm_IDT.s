.global unhandled_interrupt
.global load_idt

unhandled_interrupt:
	push %ds
    push %es
    push %fs
	pusha

	/*
		Here, at some point, we will have to go kernel mode.
		Also, I can't pop GS.
		See: https://github.com/mkilgore/protura/blob/master/arch/x86/kernel/irq_handler.S
	*/

	push %esp
	call unhandled_interrupt_handler
	add $4, %esp

	popa
    pop %fs
    pop %es
	pop %ds
	iret

load_idt:
	mov 0x4(%esp), %eax
	lidt (%eax)
	ret

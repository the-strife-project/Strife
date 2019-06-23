.global go_paging

go_paging:
	push %ebp
	mov %esp, %ebp

	mov 0x8(%esp), %eax
	mov %eax, %cr3

	mov %cr0, %eax
	or %eax, 0x80000000
	mov %eax, %cr0

	mov %ebp, %esp
	pop %ebp
	ret

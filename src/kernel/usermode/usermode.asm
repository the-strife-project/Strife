global jump_usermode

; Lower two bits set for ring 3
%define USER_DATASEGMENT (0x20 | 0b11)

; Receives the address of the MSS in the stack.
jump_usermode:
	add esp, 4	; Ignore return address.

	mov ax, USER_DATASEGMENT
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	; 'ss' is handled by iret

	; Restore registers.
	pop eax
	pop ebx
	pop ecx
	pop edx
	pop esi
	pop edi
	pop ebp

	; iretValues is at the top of the stack now. Let's go!
	iret

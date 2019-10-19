global jump_usermode

; Lower two bits set for ring 3
%define USER_CODESEGMENT (0x18 | 0b11)
%define USER_DATASEGMENT (0x20 | 0b11)

; Receives the address of the MSS in the stack.
jump_usermode:
	mov ebx, dword [esp+4]

	mov ax, USER_DATASEGMENT
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	; 'ss' is handled by iret

	mov eax, esp
	push USER_DATASEGMENT
	push eax
	pushf

	push USER_CODESEGMENT
	push ebx
	mov ebp, ebx	; Let the program know where it's loaded. Temporal. This should be fixed.
	iret

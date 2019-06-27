global go_paging

go_paging:
	push ebp
	mov ebp, esp

	mov eax, dword [esp+8]
	mov cr3, eax

	mov eax, cr0
	or dword [0x80000000], eax
	mov cr0, eax

	mov esp, ebp
	pop ebp
	ret

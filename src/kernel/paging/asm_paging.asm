global go_paging
global switch_page_table
global goback_paging

; Enable paging
go_paging:
	push ebp
	mov ebp, esp

	mov eax, dword [esp+8]
	mov cr3, eax

	mov eax, cr0
	or eax, 0x80000000
	mov cr0, eax

	mov esp, ebp
	pop ebp
	ret

; Switch page table
switch_page_table:
	mov eax, dword [esp+4]
	mov cr3, eax
	ret

; Disable paging
goback_paging:
	push ebp
	mov ebp, esp

	mov eax, cr0
	and eax, ~0x80000000
	mov cr0, eax

	mov esp, ebp
	pop ebp
	ret

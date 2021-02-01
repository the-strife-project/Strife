global go_paging
global goback_paging

; Enable paging
go_paging:
	mov eax, cr0
	or eax, 0x80000000
	mov cr0, eax
	ret

; Disable paging
goback_paging:
	mov eax, cr0
	and eax, ~0x80000000
	mov cr0, eax
	ret

BITS 32

%define KERNEL_DATASEGMENT 0x10

global IDT_syscall
extern syscall_handler

IDT_syscall:
	push ds
    push es
    push fs
	push gs
	pushad

	; The parameters.
	push ecx
	push edx
	push esi
	push edi
	push eax

	; Change the segments.
	mov ax, KERNEL_DATASEGMENT
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax

	call syscall_handler

	; Yeet 'em
	lea ebx, [5 * 4]
	add esp, ebx

	; Overwrite eax.
	mov dword [esp + (7*4)], eax

	popad
	pop gs
    pop fs
    pop es
	pop ds
	iret

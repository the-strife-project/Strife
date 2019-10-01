; Second stage of JBoot, HDD version.

BITS 16
ORG 0x8C00

; Reference some methods: print and readsector.
; This is some serious preprocessor magic. Basically "includes" the methods
; without including the code. I made 'nasmPP.py' just so I can do this.
%reference "stage1.asm"

jmp start

checkA20:
	; Source: https://wiki.osdev.org/A20_Line
	pushf
	push ds
	push es
	push di
	push si
	cli
	xor ax, ax
	mov es, ax
	not ax
	mov ds, ax
	mov di, 0x0500
	mov si, 0x0510
	mov al, byte [es:di]
	push ax
	mov al, byte [ds:si]
	push ax
	mov byte [es:di], 0x00
	mov byte [ds:si], 0xFF
	cmp byte [es:di], 0xFF
	pop ax
	mov byte [ds:si], al
	pop ax
	mov byte [es:di], al
	mov ax, 0
	jz checkA20_exit
	mov ax, 1
	checkA20_exit:
	pop si
	pop di
	pop es
	pop ds
	popf
	ret

noa20 db "A20 could not be enabled.", 0
loading db "Loading kernel...", 0x0A, 0x0D, 0x00
booting db "Booting...", 0x0A, 0x0D, 0x00
nomem db "BIOS does not support memory detection!", 0
memno20 db "BIOS returns memory detection with 24 bytes. This has never been seen!", 0

start:
; Now, check whether A20 is enabled.
call checkA20
test ax, ax
jnz A20_ENABLED

; It's not enabled. Enable it through "Fast A20 Gate".
in al, 0x92
or al, 2
out 0x92, al

; Check if it's enabled now.
call checkA20
test ax, ax
jnz A20_ENABLED

; It didn't work. Too bad for the user.
mov si, noa20
call print
jmp $

A20_ENABLED:
; A20 is enabled at this point.

; Enter Big Unreal Mode (to write past the 1M barrier).
; Thanks to: https://wiki.osdev.org/Unreal_Mode
cli	; Disable interrupts
push ds	; Save real mode
lgdt [gdtinfo]	; Load the temporal GDT

mov eax, cr0	; Switch to protected mode
or al, 1
mov cr0, eax

jmp $+2	; Tell 386/486 to not crash (y tho?)

mov bx, 0x08	; Select descriptor 1
mov ds, bx

and al, 0xFE	; Back to real mode
mov cr0, eax
pop ds
; We are now in Big Unreal Mode.

; About to begin the real shit.
mov si, loading
call print

; Bear in mind that we can't read the ELF directly, as BIOS interrupts run on real mode.

; Instead, we'll be loading one sector at a time (512B) to 0x9000.
; Some "dapack" variables are also exported from "jboot.asm".
; This is very convenient.
mov word [dapack_blkcount], 1
mov word [dapack_boffset], 0x9000

; Load the ELF at 2M, and then parse it and load the kernel at 1M.

; Get the size of the ELF.
; In JBoot-HDD, it's already there, we have the second sector in memory.
push dword [0x8000 + 0x0C]
; And save the starting sector too.
push dword [0x8000 + 0x08]

; Offsetless count.
xor ecx, ecx
LOAD_KERNEL:
	; Compute starting LBA for current block and read it.
	mov eax, [esp]	; Offset
	add eax, ecx	; + current offsetless LBA
	call readsector

	; Calculate the current block's starting position.
	mov eax, ecx	; Current offsetless LBA
	shl eax, 9	; * 512
	add eax, 0x200000	; + 2M

	; Move the block. I couldn't get "movs" working.
	push ecx
	xor ecx, ecx
	.LOAD_KERNEL_L:
		; Get current doubleword.
		mov ebx, ecx
		add ebx, 0x9000
		mov ebx, dword [ebx]

		; Move it.
		mov edx, ebx
		mov ebx, eax
		add ebx, ecx
		mov dword [ebx], edx

		; Go for the next one.
		add ecx, 4
		cmp ecx, 2048
		jl .LOAD_KERNEL_L
	pop ecx

	inc ecx
	cmp ecx, dword [esp+4]
	jl LOAD_KERNEL

; The whole ELF is now in memory!
mov si, booting
call print

; Get the kernel what it needs. We'll put all of this at 0x9000.
; 0x9000: boot drive ID (byte)
mov dl, [bootDriveID]	; Reference to stage 1
mov byte [0x9000], dl

; 0xA000: available RAM (dword)
; We'll do it by BIOS function 0x15, eax=0xE820
mov di, 0xA000
mov eax, 0xE820
xor ebx, ebx
mov ecx, 24
mov edx, 0x534D4150
int 0x15

; Check if everything went fine (BIOS supports it).
jc BIOS_NO_MEM
cmp eax, 0x534D4150
jnz BIOS_NO_MEM
cmp cl, 20
jnz BIOS_MEM_NO20

; Go for the next entries.
MEM_L:
	; Are we done?
	test ebx, ebx
	jz MEM_FINISHED

	; Nope. Go for the next one.
	mov ax, di
	xor ch, ch
	add ax, cx
	mov di, ax

	mov eax, 0xE820
	mov ecx, 24
	int 0x15
	jmp MEM_L
MEM_FINISHED:
; The list is now at 0xA000.

; Enter protected mode
mov eax, cr0
or al, 1
mov cr0, eax

mov ax, 0x08	; Select descriptor 1
mov ds, ax
mov es, ax
mov fs, ax
mov gs, ax
mov ss, ax

; Everything set. Now it's time to parse the ELF.
; jotadOS is x86, so I'll follow those specs.
; Guidance: https://wiki.osdev.org/ELF

; First, get the number of entries in the program header table (offset +44, byte)
mov ebx, 0x20002C
mov dl, byte [ebx]
push dx	; Save it in the stack
; Now, the size of each one is 32 bits. Because the ELF contains 32 bit instructions.
; (According to specs).

; Get the start of the program header.
mov ebx, 0x20001C
mov ebx, dword [ebx]
push ebx

; Iterate thru each one
xor dh, dh
PHT:
	xor eax, eax
	mov al, dh	; Current entry
	shl eax, 5	; * 32 (size)
	add eax, dword [esp]	; + Start
	add eax, 0x200000	; + Memory offset
	mov ebx, eax
	; It's now at ebx.

	; Check that the type of segment is 1 (load).
	mov eax, dword [ebx]
	cmp eax, 1
	jnz .PHT_ignore

	; At this point, type type is 1.
	; We have to "copy p_filesz bytes from p_offset to p_vaddr".

	; Get "p_offset" (offset +4, dword) into the stack.
	add ebx, 4
	mov eax, dword [ebx]	; p_offset
	add eax, 0x200000	; + Memory offset
	push eax

	; Get "p_vaddr" (offset +8, dword) as well.
	add ebx, 4
	mov eax, dword [ebx]	; p_vaddr
	push eax

	; Finally, "p_filesz" (offset +16, dword).
	add ebx, 8
	mov eax, dword [ebx]
	push eax

	; Move the data!
	push dx
	xor ecx, ecx
	.MOVE_KERNEL_L:
		; Get current doubleword.
		mov ebx, [esp+10]
		add ebx, ecx
		mov ebx, dword [ebx]

		; Move it.
		mov eax, ebx
		mov ebx, dword [esp+6]
		add ebx, ecx
		mov dword [ebx], eax

		; Go for the next one.
		add ecx, 4
		cmp ecx, dword [esp+2]
		jl .MOVE_KERNEL_L
	pop dx
	add esp, 12

	.PHT_ignore:
	; Next one!
	inc dh
	cmp dh, dl
	jl PHT

; Everything set. Use the fake multiboot header (at the beginning) to locate
; the entry point ('_start').

jmp (codedesc - gdt):protectedMode

protectedMode:
BITS 32
mov ebx, 0x100000
mov eax, dword [ebx]
jmp eax

BITS 16
BIOS_NO_MEM:
	mov si, nomem
	call print
	jmp $
BIOS_MEM_NO20:
	mov si, memno20
	call print
	jmp $

gdtinfo:
	dw gdt_end - gdt - 1	; Size of the table
	dd gdt	; Its start
gdt dd 0, 0
flatdesc db 0xff, 0xff, 0, 0, 0, 10010010b, 11001111b, 0
codedesc db 0xff, 0xff, 0, 0, 0, 10011010b, 11001111b, 0
gdt_end:

kernelbin db "KERNEL.BIN", 0x3B, "1"
kernelbin_len equ ($ - kernelbin)

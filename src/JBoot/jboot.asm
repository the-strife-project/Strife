; This is the bootloader for jotadOS.
; I'd rather not use grub, as it's quite big.
; The purpose is to make this REALLY simple.

BITS 16
ORG 0x7C00

; First, save the boot drive ID, which is in DL.
; This way, we will know where we're booting from.
; We will only use that drive.
push dx

jmp start

; Some neat functions and constants.
print:
	mov ah, 0x0E
	xor bh, bh
	.print_L:
		lodsb
		test al, al
		jz .print_end
		int 0x10
		jmp .print_L
	.print_end:
	ret

dapack:
    dapack_size:            db  0x10
    dapack_null:            db  0x00
    dapack_blkcount:        dw  0x0004
    dapack_boffset:         dw  0x8000
    dapack_bsegment:        dw  0x0000
    dapack_start:           dd  0x00000000
    dapack_upper_lba_bits:  dd  0x00000000

readsector:
	; Input:  eax = LBA
	; Output: sector at 0x8000
	mov dword [dapack_start], eax

	; Invoke the interrupt
	mov ah, 0x42
	mov dl, [esp+2]
	xor bx, bx
	mov ds, bx
	mov si, dapack
	int 0x13

	ret

welcome db "JBoot", 0x0A, 0x0D, 0x00
nolba db "BIOS lacks support for LBA addressing.", 0x00
nokernel db "Kernel not found!", 0x00

start:
; Clear screen.
mov ax, 0x0003
int 0x10

; Print welcome.
mov si, welcome
call print

; -1. Check if LBA is supported by the BIOS.
mov ah, 0x41
mov bx, 0x55AA
;mov dx, [esp]
int 0x13
jc lba_not_supported
cmp bx, 0xAA55
jnz lba_not_supported

; LBA is supported at this point.

; Load Primary Volume Descriptor onto memory.
mov eax, 0x10
PVML:
	call readsector
	mov bl, [0x8000]
	cmp bl, 0x01
	jz PVML_FOUND
	inc eax
	jmp PVML
PVML_FOUND:
; PVM is now @ 0x8000

; Load the root directory.
mov bx, 0x8000	; Base
add bx, 156	; Directory Record of root
add bx, 2	; LBA
mov eax, dword [bx]
call readsector

; Find "kernel.bin".
mov bx, 0x8000
FIND_KERNEL:
	; Get the size (al)
	mov al, [bx]
	; If it's zero, the kernel is not there.
	test al, al
	jz KERNEL_NOT_FOUND

	; Check if filename length matches.
	mov ah, [bx+32]
	cmp ah, kernelbin_len
	; If they don't match, keep seeking.
	jnz .FIND_KERNEL_KEEP

	; At this point, they do match. Now compare the strings.
	call CHECK_KERNEL
	test ax, ax
	jz KERNEL_FOUND

	; The filenames don't match.
	.FIND_KERNEL_KEEP:
	xor ah, ah
	add bx, ax
	jmp FIND_KERNEL

KERNEL_NOT_FOUND:
	mov si, nokernel
	call print
	jmp $

CHECK_KERNEL:
	; Compares the filename to kernelbin.
	; Returns 0 in case of success, 1 otherwise.
	pusha
	add bx, 33
	mov ax, bx
	; ax is the base of the string in the CD.

	xor cx, cx
	.CHECK_KERNEL_L:
		; First character.
		mov bx, kernelbin
		add bx, cx
		mov dh, [bx]
		; Second character.
		mov bx, ax
		add bx, cx
		mov dl, [bx]
		; Compare.
		cmp dh, dl
		jnz .CHECK_KERNEL_FAILURE

		; At this point, the characters match.
		; Are we done?
		inc cx
		cmp cx, kernelbin_len
		jz .CHECK_KERNEL_SUCCESS	; Yes
		jmp .CHECK_KERNEL_L	; Nope

		.CHECK_KERNEL_FAILURE:
		popa
		xor ax, ax
		inc ax
		ret
		.CHECK_KERNEL_SUCCESS:
		; We're done.
		popa
		xor ax, ax
		ret

KERNEL_FOUND:
; We have the directory record of the kernel @ bx
; Save it for later.
push bx

; TODO: load a small chunk and move it to 1M.
add bx, 10	; Data length (size)
mov ebx, [bx]
shr ebx, 9
shr ebx, 2
inc ebx
mov [dapack_blkcount], bx
pop bx
add bx, 2	; LBA
mov eax, [bx]
mov si, ax
call readsector

mov si, 0x8000
call print

jmp $

lba_not_supported:
	mov si, nolba
	call print
	jmp $

kernelbin db "KERNEL.BIN", 0x3B, "1"
kernelbin_len equ ($ - kernelbin)

times 510-($-$$) db 0
dw 0xAA55

; This will basically prepare the filesystem and some functions.
; Then, it will load the stage 2.

BITS 16
ORG 0x7C00

%define INODE_ADDRESS 0xF000
%define INODE_NBLOCKS 20
%define INODE_DBP0 24

; First, save the boot drive ID, which is in DL.
; This way, we will know where we're booting from.
; We will only use that drive.
mov [bootDriveID], dl

jmp start

; Some neat functions and constants.
%export print
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

%export bootDriveID
bootDriveID: db 0

%export dapack_boffset
dapack:
    dapack_size:            db  0x10
    dapack_null:            db  0x00
    dapack_blkcount:        dw  0x0001	; 1 block = 1 sector (512B in HDDs)
    dapack_boffset:         dw  0x8000
    dapack_bsegment:        dw  0x0000
    dapack_start:           dd  0x00000000
    dapack_upper_lba_bits:  dd  0x00000000

%export readsector
readsector:
	; Input:  eax = LBA
	; Output: blocks starting at dapack_boffset
	mov dword [dapack_start], eax

	; Invoke the interrupt
	mov ah, 0x42
	mov dl, [bootDriveID]
	xor bx, bx
	mov ds, bx
	mov si, dapack
	int 0x13
	ret

; Megasimplified version. Handles max of 5K.
readwholefile:
	; Input: eax = LBA of the inode
	; Output: blocks starting at dapack_boffset

	; Load the inode at 0xF000, which is a random address that I pulled
	; out of thin air. It just can't be 'dapack_start'.
	mov bx, word [dapack_boffset]
	push bx
	mov bx, INODE_ADDRESS
	mov word [dapack_boffset], bx
	call readsector
	pop bx
	mov word [dapack_boffset], bx

	; Get size in blocks in edx.
	mov edx, [INODE_ADDRESS+INODE_NBLOCKS]

	xor ecx, ecx
	.readwholefile_L:
		; We know the stage 2 is smaller than 5K, so we can just use the DBPs.
		; 0xF000 + 24 (DBP 0)
		lea bx, [INODE_ADDRESS + INODE_DBP0 + (ecx*4)]
		mov eax, dword [bx]

		push dx
		call readsector
		pop dx

		add dword [dapack_boffset], 512

		inc cx
		cmp cx, dx
		jl .readwholefile_L
	ret

welcome db "JBoot", 0x0A, 0x0D, 0x00
nolba db "BIOS lacks support for LBA addressing.", 0x00
signaturebad db "Bad disk signature.", 0x00

start:
; Clear screen.
mov ax, 0x0003
int 0x10

; Print welcome.
mov si, welcome
call print

; Check if LBA is supported by the BIOS.
mov ah, 0x41
mov bx, 0x55AA
int 0x13
jc lba_not_supported
cmp bx, 0xAA55
jnz lba_not_supported

; LBA is supported at this point.

; Load the superblock onto memory.
mov eax, 1
call readsector

; Check the signature
mov eax, [0x8000+0]
cmp eax, 0xEBACA000
jnz signature_BAD
mov eax, [0x8000+4]
cmp eax, 0x000CACAD
jnz signature_BAD

; Get the JBoot inode.
; Should be n_inodes + 0. But n_inodes is reserved, and it's 2.
; So it's just the second LBA sector.
mov eax, 2
call readwholefile

; Here we go!
jmp 0x8000

lba_not_supported:
	mov si, nolba
	call print
	jmp $

signature_BAD:
	mov si, signaturebad
	call print
	jmp $

times 510-($-$$) db 0
dw 0xAA55

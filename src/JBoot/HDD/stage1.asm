; This will basically prepare the filesystem and some functions.
; Then, it will load the stage 2.

; THIS IS 100% NOT FINISHED. NEITHER IS HDD'S stage2.asm

BITS 16
ORG 0x7C00

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

%export dapack_blkcount
%export dapack_boffset
dapack:
    dapack_size:            db  0x10
    dapack_null:            db  0x00
    dapack_blkcount:        dw  0x0001	; 1 block = 1 sector (512B in ATA)
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

welcome db "JBoot", 0x0A, 0x0D, 0x00
nolba db "BIOS lacks support for LBA addressing.", 0x00

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

; Load the second sector onto memory.
mov eax, 1
call readsector
; Get the beginning sector and the number of them.
mov eax, [dapack_boffset]
push dword [eax]
mov eax, [eax + 4]
; Load stage 2.
mov word [dapack_boffset], 0x8C00
mov [dapack_blkcount], eax
pop eax
call readsector

; Here we go!
jmp 0x8C00

lba_not_supported:
	mov si, nolba
	call print
	jmp $

times 510-($-$$) db 0
dw 0xAA55

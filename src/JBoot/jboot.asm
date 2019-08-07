; This will basically prepare the filesystem and some functions.
; Then, it will load the stage 2.

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
    dapack_blkcount:        dw  0x0001	; 1 block = 1 sector (2K in ATAPI)
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

filenameLength dw 1
filename dw 1

%export findfile
findfile:
	; Input:  [esp+2] = directory record address
	;         [esp+4] = length of filename (only low byte used)
	;         [esp+6] = filename*
	; Output: In case of success: ax = 0, bx = (start of directory record)
	;         In case of failure: ax = 1

	; First, save the data in the stack
	mov bx, [esp+2]
	mov ax, [esp+4]
	mov [filenameLength], ax
	mov ax, [esp+6]
	mov [filename], ax

	.findfile_L:
		; Get the size (al)
		mov al, [bx]
		; If it's zero, the kernel is not there.
		test al, al
		jz .findfile_notfound

		; Check if filename length matches.
		mov ah, [bx+32]
		cmp ah, byte [filenameLength]
		; If they don't match, keep seeking.
		jnz .findfile_keep

		; At this point, they do match. Now compare the strings.
		call findfile_check
		test ax, ax
		jz findfile_found

		; The filenames don't match.
		.findfile_keep:
		mov al, [bx]
		xor ah, ah
		add bx, ax
		jmp .findfile_L

	.findfile_notfound:
		; Not found. ax = 1
		xor ax, ax
		inc ax
		ret

	findfile_check:
		; Compares the filename.
		; Returns 0 in case of success, 1 otherwise.
		pusha
		add bx, 33
		mov ax, bx
		; ax is the base of the string in the CD.

		xor cx, cx
		.findfile_check_L:
			; First character.
			mov bx, [filename]
			add bx, cx
			mov dh, [bx]
			; Second character.
			mov bx, ax
			add bx, cx
			mov dl, [bx]
			; Compare.
			cmp dh, dl
			jnz .findfile_check_fail

			; At this point, the characters match.
			; Are we done?
			inc cx
			cmp cx, word [filenameLength]
			jz .findfile_check_success	; Yes
			jmp .findfile_check_L	; Nope

			.findfile_check_fail:
			popa
			xor ax, ax
			inc ax
			ret
			.findfile_check_success:
			; We're done.
			popa
			xor ax, ax
			ret

	findfile_found:
		; Found. ax = 0
		xor ax, ax
		ret

%export LOAD_PVD
LOAD_PVD:
	mov eax, 0x10
	PVD_L:
		call readsector
		mov bx, [dapack_boffset]
		mov bl, [bx]
		cmp bl, 0x01
		jz PVD_FOUND
		inc eax
		jmp PVD_L
	PVD_FOUND:
	ret

welcome db "JBoot", 0x0A, 0x0D, 0x00
nolba db "BIOS lacks support for LBA addressing.", 0x00
noboot db "Boot directory could not be found.", 0x00
nostage2 db "boot.bin not found!", 0x00

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

; Load Primary Volume Descriptor onto memory.
call LOAD_PVD
; PVD is now @ 0x8000

; Load the root directory.
mov bx, 0x8000	; Base
add bx, 156	; Directory Record of root
add bx, 2	; LBA
mov eax, dword [bx]
call readsector

; Find boot directory.
push boot
push boot_len
push 0x8000
call findfile
add esp, 6
test ax, ax
jz continue

; No boot directory.
mov si, noboot
call print
jmp $

continue:
; Directory record address @ bx.
; Load boot directory record.
mov word [dapack_blkcount], 0x0001	; The boot directory is not so big.
add bx, 2	; Extent
mov eax, [bx]
call readsector

; Now, find the second stage of JBoot.
push stage2
push stage2_len
push 0x8000
call findfile
add esp, 6
test ax, ax
jz continue2

; File could not be found.
mov si, nostage2
call print
jmp $

continue2:
; The second stage is no bigger than 2K, so read only 1 sector.
; We will store this at 0x8C00. We want to keep the extent of the boot directory
; where it is so we don't have to search for the kernel later.
mov word [dapack_boffset], 0x8C00
add bx, 2	; LBA
mov eax, [bx]
call readsector

; Here we go!
jmp 0x8C00

lba_not_supported:
	mov si, nolba
	call print
	jmp $

boot db "BOOT"
boot_len equ ($ - boot)

stage2 db "STAGE2.BIN", 0x3B, "1"
stage2_len equ ($ - stage2)

times 510-($-$$) db 0
dw 0xAA55

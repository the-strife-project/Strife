BITS 32
mov esp, ebp

mov eax, 1
xor edi, edi
lea esi, [ebp+welcome]
mov edx, welcome_sz
int 0x69

mov eax, 0xCACABACA
int 0x69
jmp $

welcome db "Main System Shell loading...", 0x0A, 0x0A
welcome_sz equ $ - welcome

[bits 16]
[org 0]

mov ax, cs
mov es, ax
mov di, 0x1000
mov ebx, 0x0
mov edx, 0x534D4150
mov ecx, 0x00000024
mov eax, 0x0000E820
int 0x15
mov ecx, eax
xor eax, eax
int 0xff
jmp 0x0000

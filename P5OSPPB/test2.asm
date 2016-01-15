[org 0]
[bits 16]
mov al, 0x03
int 0x10
xor ax, ax
int 0xFF


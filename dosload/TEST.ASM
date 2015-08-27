org 100h

mov bx, cs
cmp bx, 0
jz zero
mov ah, 9
mov dx, nz_msg
int 0x21
jmp end
zero:
mov ah, 9
mov dx, z_msg
int 0x21
end:
mov ah, 0x4C
int 0x21

nz_msg: DB "CS is not zero.",0xA,0xD,"$"
z_msg: DB "Yup, CS is zero.",0xA,0xD,"$"

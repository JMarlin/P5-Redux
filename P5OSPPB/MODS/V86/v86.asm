[ORG 0x0]
[BITS 16]

;This is just to see if our V86 machine plays nice with 
;our other processes

;Random test to see if calling a VESA interrupt works
mov ax, ds
mov es, ax
mov ax, 0x1000
mov si, ax
mov ax, 0x4F00
int 0x10

wait_msg: 
    mov ax, 0x2
    int 0xFF
    cmp ax, 0
    jne send_msg
    jmp wait_msg
    
send_msg:
    mov ax, 0x1
    ;Don't need to set bx because it should still
    ;contain the value set by the get message call
    mov cx, 0x1 ;This command don't mean shit
    mov dx, 0xBEEF
    int 0xFF
    jmp wait_msg

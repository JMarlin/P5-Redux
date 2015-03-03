[ORG 0x0]
[BITS 16]

;This is just to see if our V86 machine plays nice with 
;our other processes

jmp start

_msg: dw 0xBEEF
_client: dw 0x0

;Random test to see if calling a VESA interrupt works
start:
    mov ax, 0x8000
    mov ds, ax
    
wait_msg: 
    mov ax, 0x2
    int 0xFF
    cmp ax, 0
    jne decode_msg
    jmp wait_msg
    
decode_msg:
    cmp cx, 0x0
    je beef_msg
    cmp cx, 0x1
    je get_modes
    jmp wait_msg

get_modes:
    mov [_client], bx
    
    mov ax, ds
    mov es, ax
    mov ax, 0x2000
    mov di, ax
    mov al, 'V'
    mov [0x2000], al
    mov al, 'E'
    mov [0x2001], al
    mov al, 'S'
    mov [0x2000], al
    mov al, 'A'
    mov [0x2000], al    
    mov ax, 0x4F00
    int 0x10
    cmp ax, 0x004F
    je ret_msgs

    mov ax, 0xDEAD
    mov [_msg], ax
    
 ret_msgs:
    mov bx, [_client]
    mov cx, 0 ;doesn't matter
    mov dx, es
    mov ax, 0x1
    int 0xFF
    mov dx, 0x2022 ;We should get this proper in the future, but fuck it for now
    mov ax, 0x1
    int 0xFF
    jmp wait_msg
    
beef_msg:
    mov ax, 0x1
    ;Don't need to set bx because it should still
    ;contain the value set by the get message call
    mov cx, 0x1 ;This command don't mean shit
    mov dx, [_msg]
    int 0xFF
    jmp wait_msg

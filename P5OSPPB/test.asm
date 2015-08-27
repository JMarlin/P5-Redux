[bits 16]
[org 0]

;This is a test to print our state and hang so that we can see how re-entry is
;clobbering or not clobbering our regs

mov [_ax], ax
mov [_bx], bx
mov [_cx], cx
mov [_dx], dx
call get_ip
mov [_ip], ax
mov ax, ss
mov [_ss], ax
mov ax, ds
mov [_ds], ax
mov ax, cs
mov [_cs], ax
mov ax, es
mov [_es], ax
mov ax, fs
mov [_fs], ax
mov ax, gs
mov [_gs], ax
mov ax, sp
mov [_sp], ax
mov ax, si
mov [_si], ax
mov ax, bp
mov [_bp], ax
mov ax, di
mov [_di], ax
pushf
popa
mov [_flags], ax

mov si, _ax
mov di, _ax_str
mov cx, _ax
add cx, 16
shl cx, 1
nextstr:
    mov dx, [di]
    call printstr
    mov ax, [si]
    call print_hex_word
    add di, 5
    add si, 2
    cmp si, cx
    je here
    jmp nextstr
here: jmp here

_ax: dw 0
_bx: dw 0
_cx: dw 0
_dx: dw 0
_ss: dw 0
_ds: dw 0
_cs: dw 0
_es: dw 0
_fs: dw 0
_gs: dw 0
_sp: dw 0
_si: dw 0
_bp: dw 0
_di: dw 0
_ip: dw 0
_flags: dw 0

_ax_str: db " ax:", 0
_bx_str: db " bx:", 0
_cx_str: db " cx:", 0
_dx_str: db " dx:", 0
_ss_str: db " ss:", 0
_ds_str: db " ds:", 0
_cs_str: db " cs:", 0
_es_str: db " es:", 0
_fs_str: db " fs:", 0
_gs_str: db " gs:", 0
_sp_str: db " sp:", 0
_si_str: db " si:", 0
_bp_str: db " bp:", 0
_di_str: db " di:", 0
_ip_str: db " ip:", 0
_fl_str: db " fl:", 0

get_ip:
    mov bx, sp
    mov ax, [bx]
    ret

;===============================================================================
; PRINTCHAR
;===============================================================================
;;Prints the char in al
printchar:

    push bx
    push ax
    mov bh, 0x0F
    mov ah, 0x0E
    mov bl, 0
    int 0x10
    pop ax
    pop bx
    ret


;===============================================================================
; PRINTSTR
;===============================================================================
;;Prints the ASCIIZ string pointed to by edx
printstr:

    push ax
    push si
    mov si, dx

    .top:
        mov al, [si]
        call printchar
        inc si
        cmp byte [si], 0
        je .end
        jmp .top

    .end:
        pop si
        pop ax
        ret

;===============================================================================
; HEX2CHAR
;===============================================================================
;; Converts the low nybble in al to its corresponding ASCII character and
;; returns that value in al
hex2char:

    ;;See if the value is over ten
    and al, 0xF    ;Isolate the low nybble
    cmp al, 0xA
    jl .under_ten

    ;;If the value is over nine, subtract ten, add 'A' and return
    sub al, 0xA
    add al, 'A'
    ret

    ;;If the value is under ten, simply add '0' and return
    .under_ten:
        add al, '0'
        ret


;===============================================================================
; PRINT_HEX_CHAR
;===============================================================================
;; Takes a value in al and prints its hex value to the screen
print_hex_char:

    push ax
    push ax
    shr al, 4
    call hex2char
    call printchar
    pop ax
    call hex2char
    call printchar
    pop ax
    ret


;===============================================================================
; PRINT_HEX_WORD
;===============================================================================
;; Takes a value in ax and prints its hex value to the screen
print_hex_word:

    push ax
    push ax
    mov al, ah
    call print_hex_char
    pop ax
    call print_hex_char
    pop ax
    ret
;===============================================================================

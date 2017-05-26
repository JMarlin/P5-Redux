.globl _mc_src
.globl _mc_dst
.globl _mc_cnt

_mc_src: .int 0x0
_mc_dst: .int 0x0
_mc_cnt: .int 0x0

.globl _asm_memcpy
_asm_memcpy:
    pusha
    cld
    mov %ds, %ax
    mov %ax, %es
    mov _mc_src, %ecx
    and $0xFFFFFFFC, %ecx
    mov %ecx, %esi
    mov _mc_dst, %ecx
    and $0xFFFFFFFC, %ecx
    mov %ecx, %edi
    mov _mc_cnt, %ecx
    shr $2, %ecx 
    rep movsl
    popa
    ret

.globl _asm_memcpy_rev
_asm_memcpy_rev:
    pusha
    std
    mov %ds, %ax
    mov %ax, %es
    mov _mc_src, %ecx
    and $0xFFFFFFFC, %ecx
    mov %ecx, %esi
    mov _mc_dst, %ecx
    and $0xFFFFFFFC, %ecx
    mov %ecx, %edi
    mov _mc_cnt, %ecx
    shr $2, %ecx 
    rep movsl
    popa
    ret
    
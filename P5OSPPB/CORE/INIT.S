.extern _main
.section .init
.globl _start
_start:
        jmp _main
pkgoffset:
        .byte 0x00, 0x00, 0x00, 0x00
imagename:
        .asciz "P5OS R0"

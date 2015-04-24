.extern main
.section .init
.globl _start
.globl _pkgoffset
.globl _imagename
_start:
        jmp main
_pkgoffset:
        .byte 0x00, 0x00, 0x00, 0x00
_imagename:
        .asciz "P5OS R0"

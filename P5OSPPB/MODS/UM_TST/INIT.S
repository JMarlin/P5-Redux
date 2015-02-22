.extern _main
.globl _start
.section .init
_start:
		/* this should halt the exe in its path so we can really debug */
		int $0x3
        jmp _main


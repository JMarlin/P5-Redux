#!/bin/bash

as -o ini.o ini.s --32
gcc -c -o main.o main.c -nostdlib -nostartfiles -nodefaultlibs -nostdinc -ffreestanding -m32
ld -o idle.o -T lscpt.lds ini.o main.o -melf_i386
objcopy -O binary -j .init -j .text -j .data -j .bss --set-section-flags .bss=alloc,load,contents idle.o idle.mod
cp idle.mod ../../../rampak/

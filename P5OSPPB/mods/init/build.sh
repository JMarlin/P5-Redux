#!/bin/bash

as -o ini.o ini.s --32
gcc -c -o main.o main.c -nostdlib -nostartfiles -nodefaultlibs -nostdinc -ffreestanding -m32
ld -o init.o -T lscpt.lds ini.o main.o -melf_i386 ../lib/p5.o ../lib/p5s.o
objcopy -O binary -j .init -j .text -j .data -j .bss --set-section-flags .bss=alloc,load,contents init.o init.mod
cp init.mod ../../../rampak/

#!/bin/bash

#build the image conversion tool
gcc -o imgconvert imgconvert.c

#build the font header
rm font.h
./imgconvert > font.h

as -o init.o init.s --32 -g
gcc -c -o main.o main.c -nostdlib -nostartfiles -nodefaultlibs -nostdinc -ffreestanding -m32 -g
ld -o vesa.o -T lscpt.lds init.o main.o -melf_i386 ../lib/p5.o ../lib/p5s.o -g
objcopy -O binary -j .init -j .text -j .data -j .bss --set-section-flags .bss=alloc,load,contents vesa.o vesa.mod
cp vesa.mod ../../../rampak/

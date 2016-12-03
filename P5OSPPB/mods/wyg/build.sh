#!/bin/bash

as -o init.o init.s --32
gcc -c -o associativearray.o associativearray.c -nostdlib -nostartfiles -nodefaultlibs -nostdinc -ffreestanding -m32
gcc -c -o button.o button.c -nostdlib -nostartfiles -nodefaultlibs -nostdinc -ffreestanding -m32
gcc -c -o context.o context.c -nostdlib -nostartfiles -nodefaultlibs -nostdinc -ffreestanding -m32
gcc -c -o desktop.o desktop.c -nostdlib -nostartfiles -nodefaultlibs -nostdinc -ffreestanding -m32
gcc -c -o list.o list.c -nostdlib -nostartfiles -nodefaultlibs -nostdinc -ffreestanding -m32
gcc -c -o listnode.o listnode.c -nostdlib -nostartfiles -nodefaultlibs -nostdinc -ffreestanding -m32
gcc -c -o object.o object.c -nostdlib -nostartfiles -nodefaultlibs -nostdinc -ffreestanding -m32
gcc -c -o styleutils.o styleutils.c -nostdlib -nostartfiles -nodefaultlibs -nostdinc -ffreestanding -m32
gcc -c -o textbox.o textbox.c -nostdlib -nostartfiles -nodefaultlibs -nostdinc -ffreestanding -m32
gcc -c -o window.o window.c -nostdlib -nostartfiles -nodefaultlibs -nostdinc -ffreestanding -m32
gcc -c -o rect.o rect.c -nostdlib -nostartfiles -nodefaultlibs -nostdinc -ffreestanding -m32
gcc -c -o main.o main.c -nostdlib -nostartfiles -nodefaultlibs -nostdinc -ffreestanding -m32
ld -o wyg.o -T lscpt.lds init.o main.o associativearray.o button.o context.o desktop.o list.o listnode.o object.o rect.o styleutils.o textbox.o window.o -melf_i386 ../lib/p5.o ../lib/p5s.o ../lib/memory.o ../lib/memorys.o ../lib/gfx.o ../lib/key.o ../lib/mouse.o 
objcopy -O binary -j .init -j .text -j .data -j .bss --set-section-flags .bss=alloc,load,contents wyg.o wyg.mod
cp wyg.mod ../../../rampak/

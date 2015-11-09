#!/bin/bash

gcc -c -o pci.o pci.c -nostdlib -nostdinc -ffreestanding -m32
gcc -c -o gfx.o gfx.c -nostdlib -nostdinc -ffreestanding -m32
gcc -c -o p5tmp.o p5.c -nostdlib -nostdinc -ffreestanding -m32
gcc -c -o memory.o memory.c -nostdlib -nostdinc -ffreestanding -m32
ld -o p5.o p5tmp.o memory.o -melf_i386
gcc -c -o wyg.o wyg.c -nostdlib -nostdinc -ffreestanding -m32
as -o p5s.o p5.s --32

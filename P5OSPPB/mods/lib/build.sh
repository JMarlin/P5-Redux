#!/bin/bash

gcc -c -o pci.o pci.c -nostdlib -nostdinc -ffreestanding -m32 -g
gcc -c -o gfx.o gfx.c -nostdlib -nostdinc -ffreestanding -m32 -g
gcc -c -o memory.o memory.c -nostdlib -nostdinc -ffreestanding -m32 -g
gcc -c -o p5.o p5.c -nostdlib -nostdinc -ffreestanding -m32 -g
gcc -c -o key.o key.c -nostdlib -nostdinc -ffreestanding -m32 -g
gcc -c -o mouse.o mouse.c -nostdlib -nostdinc -ffreestanding -m32 -g
gcc -c -o blockdev.o blockdev.c -nostdlib -nostdinc -ffreestanding -m32 -g
gcc -c -o wyg.o wyg.c -nostdlib -nostdinc -ffreestanding -m32 -g
as -o p5s.o p5.s --32 -g
as -o memorys.o memory.s --32 -g

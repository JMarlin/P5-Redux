#!/bin/bash

as -o init.o init.s --32
gcc -c -o main.o main.c -nostdlib -nostartfiles -nodefaultlibs -nostdinc -ffreestanding -m32
ld -o usr.mod -T lscpt.lds init.o main.o ../lib/p5.o ../lib/p5s.o -melf_i386
cp usr.mod ../../../rampak/



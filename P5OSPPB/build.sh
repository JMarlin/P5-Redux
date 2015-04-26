#!/bin/sh

#SERIOUSLY,THOUGH, THIS NEEDS TO NOT BE A LIST OF BUILD COMMANDS SOME TIME IN THE FUTURE

echo off

cd core
echo Starting core build...
as -o ../build/syscalls.o syscall.s --32
gcc -c -o ../build/syscall.o syscall.c -nostdlib -nostartfiles -nodefaultlibs -nostdinc -ffreestanding -m32
as -o ../build/init.o init.s --32
gcc -c -o ../build/kernel.o kernel.c -nostdlib -nostartfiles -nodefaultlibs -nostdinc -ffreestanding -m32
gcc -c -o ../build/cicomp.o cicomp.c -nostdlib -nostartfiles -nodefaultlibs -nostdinc -ffreestanding -m32
gcc -c -o ../build/commands.o commands.c -nostdlib -nostartfiles -nodefaultlibs -nostdinc -ffreestanding -m32
gcc -c -o ../build/util.o util.c -nostdlib -m32
gcc -c -o ../build/int.o int.c -nostdlib -nostartfiles -nodefaultlibs -nostdinc -ffreestanding -m32
as -o ../build/expt.o expt.s --32

cd ../memory
echo Starting memory build...
gcc -c -o ../build/memory.o memory.c -nostdlib -nostartfiles -nodefaultlibs -nostdinc -ffreestanding -m32
gcc -c -o ../build/gdt.o gdt.c -nostdlib -nostartfiles -nodefaultlibs -nostdinc -ffreestanding -m32
gcc -c -o ../build/paging.o paging.c -nostdlib -nostartfiles -nodefaultlibs -nostdinc -ffreestanding -m32
as -o ../build/pagings.o paging.s --32

cd ../obj
echo Starting obj build...
gcc -c -o ../build/lists.o lists.c -nostdlib -nostartfiles -nodefaultlibs -nostdinc -ffreestanding -m32
gcc -c -o ../build/variant.o variant.c -nostdlib -nostartfiles -nodefaultlibs -nostdinc -ffreestanding -m32

cd ../ascii_io
echo Starting ascii_io build...
gcc -c -o ../build/ascii_i.o ascii_i.c -nostdlib -nostartfiles -nodefaultlibs -nostdinc -ffreestanding -m32
gcc -c -o ../build/ascii_o.o ascii_o.c -nostdlib -nostartfiles -nodefaultlibs -nostdinc -ffreestanding -m32
gcc -c -o ../build/keyboard.o keyboard.c -nostdlib -nostartfiles -nodefaultlibs -nostdinc -ffreestanding  -m32
gcc -c -o ../build/serial.o serial.c -nostdlib -nostartfiles -nodefaultlibs -nostdinc -ffreestanding -m32

cd ../process
echo Starting process build...
as -o ../build/processs.o process.s --32
gcc -c -o ../build/process.o process.c -nostdlib -nostartfiles -nodefaultlibs -nostdinc -ffreestanding -m32
gcc -c -o ../build/message.o message.c -nostdlib -nostartfiles -nodefaultlibs -nostdinc -ffreestanding -m32

cd ../kserver
echo Starting kserver build...
gcc -c -o ../build/kserver.o kserver.c -nostdlib -nostartfiles -nodefaultlibs -nostdinc -ffreestanding -m32

cd ../fat12
echo Starting fat12 build...
gcc -c -o ../build/hiinter.o hiinter.c -nostdlib -nostartfiles -nodefaultlibs -nostdinc -ffreestanding -m32

cd ../block
echo Starting block build...
gcc -c -o ../build/block.o block.c -nostdlib -nostartfiles -nodefaultlibs -nostdinc -ffreestanding -m32
gcc -c -o ../build/ramdisk.o ramdisk.c -nostdlib -nostartfiles -nodefaultlibs -nostdinc -ffreestanding -m32

cd ../fs
echo Starting fs build...
gcc -c -o ../build/fs.o fs.c -nostdlib -nostartfiles -nodefaultlibs -nostdinc -ffreestanding -m32
gcc -c -o ../build/ramfs.o ramfs.c -nostdlib -nostartfiles -nodefaultlibs -nostdinc -ffreestanding -m32

cd ../timer
echo Starting timer build...
gcc -c -o ../build/timer.o timer.c -nostdlib -nostartfiles -nodefaultlibs -nostdinc -ffreestanding -m32
as -o ../build/timers.o timer.s --32

cd ../build
echo Linking binary
ld -o ../bin/p5kern.bin -T ../lscpt.lds -melf_i386 ../files 

cd ..
echo p5kern.bin finished

echo --------------------------------------------------------------------------------

echo   P5OSR0ppb compilation finished.
echo   Please consult README.TXT for information on using the two binaries which you
echo  just created.

echo --------------------------------------------------------------------------------


echo on

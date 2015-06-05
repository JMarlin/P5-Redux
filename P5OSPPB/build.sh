#!/bin/sh

#SERIOUSLY,THOUGH, THIS NEEDS TO NOT BE A LIST OF BUILD COMMANDS SOME TIME IN THE FUTURE

echo off

#this increases the build number in the kernel header
cd core
buildno=$(cat kernel.h | grep "#define P5_BUILD_NUMBER " | cut -d' ' -f3)
buildno=$(($buildno + 1))
echo Executing P5 build \#$buildno
sed "s\^#define P5_BUILD_NUMBER .*$\#define P5_BUILD_NUMBER $buildno\ " kernel.h > tmp_h
rm kernel.h
mv tmp_h kernel.h
cd ..

C_OPTS="-nostdlib -nostartfiles -nodefaultlibs -nostdinc -ffreestanding -m32" # -fno-zero-initialized-in-bss"

cd core
echo Starting core build...
as -o ../build/syscalls.o syscall.s --32
gcc -c -o ../build/syscall.o syscall.c $C_OPTS
as -o ../build/init.o init.s --32
gcc -c -o ../build/kernel.o kernel.c $C_OPTS
gcc -c -o ../build/cicomp.o cicomp.c $C_OPTS
gcc -c -o ../build/commands.o commands.c $C_OPTS
gcc -c -o ../build/util.o util.c -nostdlib -m32
gcc -c -o ../build/int.o int.c $C_OPTS
as -o ../build/expt.o expt.s --32

cd ../memory
echo Starting memory build...
gcc -c -o ../build/memory.o memory.c $C_OPTS
gcc -c -o ../build/gdt.o gdt.c $C_OPTS
gcc -c -o ../build/paging.o paging.c $C_OPTS
as -o ../build/pagings.o paging.s --32

cd ../obj
echo Starting obj build...
gcc -c -o ../build/lists.o lists.c $C_OPTS
gcc -c -o ../build/variant.o variant.c $C_OPTS

cd ../ascii_io
echo Starting ascii_io build...
gcc -c -o ../build/ascii_i.o ascii_i.c $C_OPTS
gcc -c -o ../build/ascii_o.o ascii_o.c $C_OPTS
gcc -c -o ../build/keyboard.o keyboard.c $C_OPTS
gcc -c -o ../build/serial.o serial.c $C_OPTS

cd ../process
echo Starting process build...
as -o ../build/processs.o process.s --32
gcc -c -o ../build/process.o process.c $C_OPTS
gcc -c -o ../build/message.o message.c $C_OPTS

cd ../kserver
echo Starting kserver build...
gcc -c -o ../build/kserver.o kserver.c $C_OPTS

cd ../fat12
echo Starting fat12 build...
gcc -c -o ../build/hiinter.o hiinter.c $C_OPTS

cd ../block
echo Starting block build...
gcc -c -o ../build/block.o block.c $C_OPTS
gcc -c -o ../build/ramdisk.o ramdisk.c $C_OPTS

cd ../fs
echo Starting fs build...
gcc -c -o ../build/fs.o fs.c $C_OPTS
gcc -c -o ../build/ramfs.o ramfs.c $C_OPTS

cd ../timer
echo Starting timer build...
gcc -c -o ../build/timer.o timer.c $C_OPTS
as -o ../build/timers.o timer.s --32

cd ../build
echo Linking binary
ld -o ../bin/p5kern.o -T ../lscpt.lds -melf_i386 ../files
objcopy -O binary -j .init -j .text -j .data -j .bss --set-section-flags .bss=alloc,load,contents ../bin/p5kern.o ../bin/p5kern.bin

cd ..
echo p5kern.bin finished

echo --------------------------------------------------------------------------------

echo   P5OSR0ppb compilation finished.
echo   Please consult README.TXT for information on using the two binaries which you
echo  just created.

echo --------------------------------------------------------------------------------

echo on

echo off
echo Compilation started...
rem echo Starting PBOOT build...
rem nasm -fbin -o pboot.img PBOOT\pboot.asm
rem echo ...PBOOT finished
cd CORE
echo Starting CORE build...
as -o init.o init.s
gcc -c -o kernel.o kernel.c -nostdlib -nostartfiles -nodefaultlibs -nostdinc -ffreestanding
gcc -c -o cicomp.o CICOMP.c -nostdlib -nostartfiles -nodefaultlibs -nostdinc -ffreestanding
gcc -c -o commands.o COMMANDS.c -nostdlib -nostartfiles -nodefaultlibs -nostdinc -ffreestanding
gcc -c -o util.o util.c -nostdlib
gcc -c -o int.o int.c -nostdlib -nostartfiles -nodefaultlibs -nostdinc -ffreestanding
echo ...CORE finished
CD ..\MEMORY
echo Starting MEMORY build...
gcc -c -o memory.o memory.c -nostdlib -nostartfiles -nodefaultlibs -nostdinc -ffreestanding
gcc -c -o paging.o paging.c -nostdlib -nostartfiles -nodefaultlibs -nostdinc -ffreestanding
as -o pagings.o paging.s
CD ..\OBJ
echo Starting OBJ build...
gcc -c -o lists.o lists.c -nostdlib -nostartfiles -nodefaultlibs -nostdinc -ffreestanding
gcc -c -o variant.o variant.c -nostdlib -nostartfiles -nodefaultlibs -nostdinc -ffreestanding
CD ..\ASCII_IO
echo Starting ASCII_IO build...
gcc -c -o ascii_i.o ASCII_I.c -nostdlib -nostartfiles -nodefaultlibs -nostdinc -ffreestanding
gcc -c -o ascii_o.o ASCII_O.c -nostdlib -nostartfiles -nodefaultlibs -nostdinc -ffreestanding
gcc -c -o keyboard.o keyboard.c -nostdlib -nostartfiles -nodefaultlibs -nostdinc -ffreestanding 
echo ...ASCII_IO finished
CD ..\FAT12
echo Starting FAT12 build...
gcc -c -o hiinter.o HIINTER.c -nostdlib -nostartfiles -nodefaultlibs -nostdinc -ffreestanding
echo ...FAT12 finished
cd ..
echo Linking binary:
ld -o p5kern.bin -T lscpt.lds files
rem ld -e _main -static -Ttext 0x1700 --oformat binary -o p5kern.bin linkscript
echo ..P5KERN.BIN finished


echo --------------------------------------------------------------------------------

echo   P5OSR0ppb compilation finished. 
echo   Please consult README.TXT for information on using the two binaries which you 
echo  just created.

echo --------------------------------------------------------------------------------


echo on


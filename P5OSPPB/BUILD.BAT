echo off
echo Compilation started...
rem echo Starting PBOOT build...
rem nasm -fbin -o pboot.img PBOOT\pboot.asm
rem echo ...PBOOT finished
cd CORE
echo Starting CORE build...
as -o ..\build\init.o init.s
gcc -c -o ..\build\kernel.o kernel.c -nostdlib -nostartfiles -nodefaultlibs -nostdinc -ffreestanding
gcc -c -o ..\build\cicomp.o CICOMP.c -nostdlib -nostartfiles -nodefaultlibs -nostdinc -ffreestanding
gcc -c -o ..\build\commands.o COMMANDS.c -nostdlib -nostartfiles -nodefaultlibs -nostdinc -ffreestanding
gcc -c -o ..\build\util.o util.c -nostdlib
echo ...CORE finished
CD ..\ASCII_IO
echo Starting ASCII_IO build...
gcc -c -o ..\build\ascii_i.o ASCII_I.c -nostdlib -nostartfiles -nodefaultlibs -nostdinc -ffreestanding
gcc -c -o ..\build\ascii_o.o ASCII_O.c -nostdlib -nostartfiles -nodefaultlibs -nostdinc -ffreestanding
gcc -c -o ..\build\keyboard.o keyboard.c -nostdlib -nostartfiles -nodefaultlibs -nostdinc -ffreestanding
echo ...ASCII_IO finished
CD ..\FAT12
echo Starting FAT12 build...
gcc -c -o ..\build\hiinter.o HIINTER.c -nostdlib -nostartfiles -nodefaultlibs -nostdinc -ffreestanding
echo ...FAT12 finished
CD ..\COFF
echo Starting COFF build...
gcc -c -o ..\build\coff.o COFF.c -nostdlib -nostartfiles -nodefaultlibs -nostdinc -ffreestanding
echo ...COFF finished
cd ..
echo Linking binary:
cd build
ld -o ..\p5kern.bin -T ..\lscpt.lds ..\files
cd ..
rem ld -e _main -static -Ttext 0x1700 --oformat binary -o p5kern.bin linkscript
echo ..P5KERN.BIN finished


echo --------------------------------------------------------------------------------

echo   P5OSR0ppb compilation finished. 
echo   Please consult README.TXT for information on using the two binaries which you 
echo  just created.

echo --------------------------------------------------------------------------------


echo on


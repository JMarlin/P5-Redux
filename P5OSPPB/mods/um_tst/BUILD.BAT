as -o init.o init.s
gcc -c -o main.o main.c -nostdlib -nostartfiles -nodefaultlibs -nostdinc -ffreestanding
ld -o usr.mod -T lscpt.lds init.o main.o ..\lib\p5.o


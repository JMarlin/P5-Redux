gcc -c -o build\modcore.o core\modcore.c -D P5_MODULE -nostdlib -nostartfiles -nodefaultlibs -nostdinc -ffreestanding
gcc -c -o build\keyboard.o keyboard\keyboard.c -D P5_MODULE -nostdlib -nostartfiles -nodefaultlibs -nostdinc -ffreestanding
ld -oformatcoff-go32 -o dellkey.mod build\*.o

nasmw -fbin p5boot.asm -o p5boot.bin
nasmw -fbin p5kern.asm -o p5kern.bin
makeboot P5OSR0.img p5boot.bin p5kern.bin 



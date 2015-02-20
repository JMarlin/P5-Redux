copy bin\p5kern.bin ..\kpkg\
cd ..\rampak\
gcc -o rampak.exe main.c
rampak p5.rd test.txt
copy p5.rd ..\kpkg\
cd ..\kpkg\
kpkg p5kern.bin p5.rd
copy p5kern.bin ..\dosload\
cd ..\dosload\
dosload.com



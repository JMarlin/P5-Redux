cp bin/p5kern.bin ../kpkg/
cd ../rampak/
gcc -o rampak main.c
#./rampak p5.rd usr.mod init.mod registrar.mod idle.mod ps2.mod fdc.mod vfs.mod
./rampak p5.rd init.mod registrar.mod
cp p5.rd ../kpkg/
cd ../kpkg/
gcc -o kpkg main.c
./kpkg p5kern.bin p5.rd
cp p5kern.bin ../dosload/
cd ../dosload/
#dosload.com

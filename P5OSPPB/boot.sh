cp bin/p5kern.bin ../kpkg/
cd ../rampak/
gcc -o rampak main.c
./rampak p5.rd test.txt usr.mod dos.mod v86.mod
cp p5.rd ../kpkg/
cd ../kpkg/
gcc -o kpkg main.c
./kpkg p5kern.bin p5.rd
#cp p5kern.bin ../dosload/
#cd ../dosload/
#dosload.com


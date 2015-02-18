copy bin\p5kern.bin ..\kpkg\
cd ..\kpkg\
kpkg p5kern.bin test.rd
copy p5kern.bin ..\dosload\
cd ..\dosload\
dosload.com



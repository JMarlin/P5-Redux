copy bin\p5kern.bin ..\kpkg\
cd ..\kpkg\
kpkg p5kern.bin blank2.drv blank.drv
copy p5kern.bin ..\dosload\
cd ..\dosload\
dosload.com



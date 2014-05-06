echo off
cls
copy p5kern.bin ..\kpkg\
cd ..\kpkg\
kpkg p5kern.bin dellkey.mod
copy p5kern.bin ..\dosload\
cd ..\dosload
echo About to boot into P5
pause
dosload

#!/bin/bash

gcc -w -c -o entry.o entry.c -I/c/minglibs/include
gcc -w -c gfx.c -o gfx.o -I/c/minglibs/include/SDL2 -I/c/minglibs/include
gcc -w -c p5.c -o p5.o -I/c/minglibs/include
gcc -w -I/c/minglibs/include/SDL2 -I/c/minglibs/include -L/c/minglibs/lib -DHARNESS_TEST=1 ../../mods/wyg/main.c ./gfx.o ./p5.o ./entry.o -o wyg_test -lSDL2
#ld -o wyg_test entry.o gfx.o p5.o wyg.o -lSDL2 -lc

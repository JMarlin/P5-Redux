#!/bin/bash

gcc -c -o entry.o entry.c `sdl2-config --cflags`
gcc -c gfx.c `sdl2-config --cflags` -o gfx.o
gcc -c p5.c -o p5.o
gcc `sdl2-config --cflags --libs` -DHARNESS_TEST=1 ../../mods/wyg/main.c ./gfx.o ./p5.o ./entry.o -o wyg_test 
#ld -o wyg_test entry.o gfx.o p5.o wyg.o -lSDL2 -lc

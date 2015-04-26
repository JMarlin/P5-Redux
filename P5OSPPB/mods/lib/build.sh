#!/bin/bash

gcc -c -o p5.o p5.c -nostdlib -nostdinc -ffreestanding -m32
as -o p5s.o p5.s --32

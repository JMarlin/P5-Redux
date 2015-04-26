#!/bin/sh

nasm -o v86.mod v86.asm -fbin
cp v86.mod ../../../rampak/

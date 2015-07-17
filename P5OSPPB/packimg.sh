#!/bin/sh

if (( $# > 0 )) ; then
    ./rollp5.sh "$1"
else
    ./rollp5.sh
fi

cd pboot
./mkpboot.sh 30

mkdir img
mount -o loop pboot.img ./img
cp ../bin/p5kern.bin ./img/
sync
umount ./img

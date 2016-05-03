#!/bin/sh

cd ./pboot/
bash ./mkfloppy.sh
cd ..
cp ./pboot/pboot_fat12.img ./
mount -o loop ./pboot_fat12.img /mnt/dos
cp ../dosload/p5kern.bin /mnt/dos/
umount /mnt/dos/
chmod a+rw ./pboot_fat12.img


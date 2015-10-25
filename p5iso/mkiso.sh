#!/bin/sh

mount -o loop pboot_fat12.img /mnt/dos
cp ../dosload/p5kern.bin /mnt/dos/
umount /mnt/dos
cp pboot_fat12.img ./cdcontent/
mkisofs -o p5.iso -V P5OS -b pboot_fat12.img cdcontent

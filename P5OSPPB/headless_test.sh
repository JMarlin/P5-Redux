#!/bin/sh
qemu-system-x86_64 -gdb tcp::1234 -nographic -fda ./pboot_fat12.img -boot a 


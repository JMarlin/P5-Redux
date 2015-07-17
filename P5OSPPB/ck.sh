if (( $# < 1 )); then
	$1=/dev/sdb
fi 

mount $1 /mnt/dos/
cp ../dosload/p5kern.bin /mnt/dos/p5kern.bin
sync
umount /mnt/dos


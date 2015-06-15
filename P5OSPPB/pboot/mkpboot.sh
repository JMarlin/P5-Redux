#!/bin/sh

#NOTE: Would be nice to be able to have this script autocalculate the number of
#sectors per FAT based on the overall size of the image
#NOTE Currently does not support images over 32MB (would require conditionally
#writing to the extended size area at the end of the BPB)

#NOTE: Input is size of final image in megabytes
TOTALSECT=(($1))
((TOTALSECT *= 2048))

#Image params
SECTORSZ=$((512))
FATCOPIES=$((2))
ROOTCLUSTERS=$((14))
SECTPERFAT=$((#CALCULATEME))
SECTPERCLUSTER=$((1))

#build the second stage bootloader sectors
nasm -o stage2.bin stage2.asm -fbin

#calculate how many sectors will be required for the second stage code
S2_SIZE=$((`cat stage2.bin | wc -c`))
S2_SECTORS=$((S2_SIZE / SECTORSZ))
S2_PAD=$((S2_SIZE % SECTORSZ))
if [ $S2_PAD -gt 0 ]
then
    ((S2_SECTORS += 1))
fi

#build the bootsector based on the calculated sizes
nasm -o bootsect.bin bootsect.asm -dTOTALSECT=$TOTALSECT -dRESSECTORS=$S2_SECTORS -dSECTORSZ=$SECTORSZ -dFATCOPIES=$FATCOPIES -dROOTCLUSTERS=$ROOTCLUSTERS -dSECTPERFAT=$SECTPERFAT -dSECTPERCLUSTER=$SECTPERCLUSTER -fbin

#concatinate the bootsector and the stage2 sectors
cat bootsect.bin stage2.bin > pboot.img

#append enough bytes to the end of the second stage blob to align to sector boundaries
dd if=/dev/zero bs=1 count=$S2_PAD >> pboot.img

#write the first set of reserved clusters for the first FAT, pad to sector size,
#then pad with remaining empty fat sectors
echo -n -e '\xF0\xFF\xFF' >> pboot.img
dd if=/dev/zero bs=1 count=509 >> pboot.img
dd if=/dev/zero bs=512 count=191 >> pboot.img

#do the same for the second FAT
echo -n -e '\xF0\xFF\xFF' >> pboot.img
dd if=/dev/zero bs=1 count=509 >> pboot.img
dd if=/dev/zero bs=512 count=191 >> pboot.img

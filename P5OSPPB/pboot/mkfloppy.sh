#!/bin/sh

#NOTE Currently does not support images over 32MB (would require conditionally
#writing to the extended size area at the end of the BPB)
#NOTE: Some day, we should totally make this support FAT16/32
#NOTE: Input is size of final image in megabytes

#Supress output
#exec > /dev/null 2>&1

#The success message which will be placed into the SUCCESS.TXT
MESSAGE="This is a fresh PBoot boot image. To make this a functional P5OS boot volume, place a valid P5 kernel image into the root of this image with the filename 'P5KERN.BIN'"

#Image params 
#We will always use a single cluster per sector because fuck you I'm lazy
SECTORSZ=$((512))
SECTPERCLUSTER=$((1))
#S2_SIZE, number of reserved sectors
FATCOPIES=$((2))
ROOTENTRIES=$((16)) #this *32 should be and exact multiple of sector size
TOTALSECT=$((2880)) #512 x 2880 = 1.44MB
SECTPERFAT=$((9))
#For FAT12, 9 sectors are required per FAT

#build the second stage bootloader sectors
nasm -o stage2_fat12.bin stage2_fat12.asm -fbin

#calculate how many sectors will be required for the second stage code
S2_SIZE=$((`cat stage2.bin | wc -c`))
S2_SECTORS=$((S2_SIZE / SECTORSZ))
S2_PAD=$((S2_SIZE % SECTORSZ))
if [ $S2_PAD -gt 0 ]; then
    ((S2_SECTORS += 1))
fi
S2_PAD=$((SECTORSZ - S2_PAD))

#build the bootsector based on the calculated sizes
nasm -o bootsect.bin bootsect.asm -dTOTALSECT=$TOTALSECT -dRESSECT=$S2_SECTORS -dSECTORSZ=$SECTORSZ -dFATCOPIES=$FATCOPIES -dROOTENTRIES=$ROOTENTRIES -dSECTPERFAT=$SECTPERFAT -dSECTPERCLUSTER=$SECTPERCLUSTER -fbin

#concatinate the bootsector and the stage2 sectors
cat bootsect.bin stage2_fat12.bin > pboot_fat12.img

#append enough bytes to the end of the second stage blob to align to sector boundaries
dd if=/dev/zero bs=1 count=$S2_PAD >> pboot.img

#Write as many FATs as specified
for ((findex=0; findex<FATCOPIES; findex++)); do

    #write the first set of reserved clusters for the FAT, pad to sector size,
    #then pad with remaining empty fat sectors
    #NOTE: The first three bytes are the default cluster0 and cluster1 values
    #cluster0 value indicating that this is an unpartitioned superfloppy and
    #cluster1 indicating end-of-chain. The next three are an end-of-chain for
    #the default file and the next clear cluster
    echo -n -e '\xF0\xFF\xFF\xFF\xFF\xFF' >> pboot.img
    dd if=/dev/zero bs=1 count=$((SECTORSZ - 6)) >> pboot.img
    dd if=/dev/zero bs=$SECTORSZ count=$((SECTPERFAT - 1)) >> pboot.img
done

#Write the root directory entry starting with the volume label
#each entry is 32 bytes of info
echo -n 'PBOOT_R2   ' >> pboot.img         #Text for volume label
echo -n -e '\x08' >> pboot.img             #Attributes: 0x08 is volume label
dd if=/dev/zero bs=1 count=20 >> pboot.img #The rest of the entry should be null

#Now the entry for the default file we include baked in
TXTLEN=$((`echo -n $MESSAGE | wc -c`))      #calculate the length of the text
TXLENS=`printf "%04x" $TXTLEN`              #turn it into a 2-byte hex string
echo -n 'SUCCESS TXT' >> pboot.img          #8.3 file name
dd if=/dev/zero bs=1 count=15 >> pboot.img  #Don't need any of these 15 bytes
echo -n -e '\x02\x00' >> pboot.img          #Start cluster, little-endian
echo -n -e `echo \\\x${TXLENS:2:2}\\\x${TXLENS:0:2}\\\x00\\\x00` >> pboot.img  #Filesize, little-endian (one sect)

#Now fill out the rest of the directory entry
dd if=/dev/zero bs=32 count=$((ROOTENTRIES - 2)) >> pboot.img

#Write the first cluster (cluster 2) with the message info
echo -n $MESSAGE >> pboot.img

#Pad the rest of the sector
TXTPAD=$((SECTORSZ - TXTLEN))
dd if=/dev/zero bs=1 count=$TXTPAD >> pboot.img

#And finally write enough empty sectors to hit the image size
#Bootsect + stage2 + fats + rootdir + cluster2
echo $((TOTALSECT - (1 + S2_SECTORS + (SECTPERFAT * FATCOPIES) + 1))) leftover
dd if=/dev/zero bs=$((SECTORSZ)) count=$((TOTALSECT - (S2_SECTORS + (SECTPERFAT * FATCOPIES) + 3))) >> pboot.img

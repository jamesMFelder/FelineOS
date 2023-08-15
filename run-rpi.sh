#!/bin/sh

set -e
mount /dev/mmcblk0p1 /mnt/
cp ${HOME}/src/FelineOS/sysroot/boot/kernel.bin /mnt/kernel.img
sleep 1
umount /mnt

#!/bin/sh
set -e
. ./build.sh

mkdir -p isodir
mkdir -p isodir/boot
mkdir -p isodir/boot/grub

cp sysroot/boot/FelineOS.kernel isodir/boot/FelineOS.kernel
cat > isodir/boot/grub/grub.cfg << EOF
menuentry "myos" {
	multiboot /boot/FelineOS.kernel
}
EOF
grub-mkrescue -o FelineOS.iso isodir -d /usr/lib/grub/i386-pc

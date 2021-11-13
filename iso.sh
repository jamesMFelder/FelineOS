#!/bin/sh
# SPDX-License-Identifier: MIT
# Copyright (c) 2021 James McNaughton Felder
set -e
#shellcheck disable=SC1091
. ./build.sh

mkdir -p isodir
mkdir -p isodir/boot
mkdir -p isodir/boot/grub

cp sysroot/boot/FelineOS.kernel isodir/boot/FelineOS.kernel
cat > isodir/boot/grub/grub.cfg << EOF
insmod vbe
insmod vga
menuentry "myos" {
	multiboot /boot/FelineOS.kernel
}
set timeout=1
EOF
grub-mkrescue -o FelineOS.iso isodir -d /usr/lib/grub/i386-pc

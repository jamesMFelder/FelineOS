#!/bin/sh
# SPDX-License-Identifier: MIT
# Copyright (c) 2021 James McNaughton Felder
set -e
#shellcheck disable=SC1091
. ./build.sh

mkdir -p isodir
mkdir -p isodir/boot
mkdir -p isodir/boot/grub

#Copy the kernel into the iso
cp sysroot/boot/FelineOS.kernel isodir/boot/FelineOS.kernel
#Create a GRUB config
cat > isodir/boot/grub/grub.cfg << EOF
#Load all graphics drivers we  can
insmod vbe
insmod vga
insmod efi_gop
insmod efi_uga
#Create a menuentry to boot the kernel
menuentry "FelineOS" {
	multiboot /boot/FelineOS.kernel
}
#Boot after 1 second if we don't do anything
set timeout=1
EOF
#Tell VirtualBox what to boot
cat > isodir/startup.nsh << EOF
FS0:\System\Library\CoreServices\boot.efi
EOF
#Create the iso
grub2-mkrescue -o FelineOS.iso isodir
#Create a symbol file for bochs
nm -Cg isodir/boot/FelineOS.kernel | sed -n 's/^\([[:xdigit:]]\+\) [[:alpha:]] /0x\1 /p' > kernel.sym

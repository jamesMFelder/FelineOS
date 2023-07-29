#!/bin/bash
# SPDX-License-Identifier: MIT
# Copyright (c) 2023 James McNaughton Felder
set -e
cd /home/james/src/FelineOS/
. ./iso.sh

if [ "$1" = "-g" ]; then
	GDB_START="-s -S"
fi

if [ "$(./target-triplet-to-arch.sh "$TARGET_HOST")" = "arm" ]; then
	EXTRA_DEVICES="-dtb bcm2708-rpi-b.dtb"
	# Load at 0x8000, where the Raspberry Pi does
	# Use cpu-num so QEMU starts executing at 0x8000 instead of 0x0
	BOOT="-device loader,file=sysroot/boot/kernel.bin,addr=0x8000,cpu-num=0"
elif [ "$(./target-triplet-to-arch.sh "$TARGET_HOST")" = "i386" ]; then
	BOOT="-cdrom FelineOS.iso -boot d"
else
	echo "Unknown architecture $(./target-triplet-to-arch.sh "$TARGET_HOST")! Cannot choose how to boot!" >&2
	exit 1
fi

#shellcheck disable=2086 # options do need word-splitting
"qemu-system-$(./target-triplet-to-arch.sh "$(./target-triplet-to-arch.sh "$TARGET_HOST")")" -M "$(./arch-machine.sh)" $BOOT $EXTRA_DEVICES -serial stdio $GDB_START

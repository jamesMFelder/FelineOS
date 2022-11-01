#!/bin/sh
# SPDX-License-Identifier: MIT
# Copyright (c) 2021 James McNaughton Felder
set -e
cd /home/james/src/FelineOS/
. ./iso.sh

if [ "$1" = "-g" ]; then
	GDB_START="-S"
fi

if [ "$(./target-triplet-to-arch.sh "$TARGET_HOST")" = "arm" ]; then
	EXTRA_DEVICES="-dtb bcm2708-rpi-b.dtb"
	BOOT="-kernel system/kernel/FelineOS.kernel"
elif [ "$(./target-triplet-to-arch.sh "$TARGET_HOST")" = "i386" ]; then
	BOOT="-cdrom FelineOS.iso"
else
	echo "Unknown architecture $(./target-triplet-to-arch.sh "$TARGET_HOST")! Cannot choose how to boot!" >&2
	exit 1
fi

#shellcheck disable=2086 # options do need word-splitting
"qemu-system-$(./target-triplet-to-arch.sh "$(./target-triplet-to-arch.sh "$TARGET_HOST")")" -M "$(./arch-machine.sh)" $BOOT $EXTRA_DEVICES -serial stdio -s $GDB_START

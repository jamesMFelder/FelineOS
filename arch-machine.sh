#!/bin/sh
# SPDX-License-Identifier: MIT
# Copyright (c) 2021 James McNaughton Felder
HOST=$(./default-host.sh)
if [ "${HOST}" = "arm-none-eabi" ] ; then
	echo "raspi1ap"
elif [ "${HOST}" = "i686-elf" ] ; then
	echo "pc"
else
	echo "Unknown host ${HOST}. Cannot supply qemu arguments." >&2
	echo "Aborting." >&2
	exit 1
fi

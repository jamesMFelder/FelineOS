#!/bin/sh
# SPDX-License-Identifier: MIT
# Copyright (c) 2021 James McNaughton Felder
HOST=$(./default-host.sh)
if [ "${HOST}" = "arm-none-eabi" ] ; then
	echo "-mcpu=arm1176jzf-s -mfloat-abi=soft"
elif [ "${HOST}" = "i686-elf" ] ; then
	echo ""
else
	echo "Unknown host ${HOST}. Cannot supply compiler arguments." >&2
	echo "Aborting." >&2
	exit 1
fi

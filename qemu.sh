#!/bin/sh
# SPDX-License-Identifier: MIT
# Copyright (c) 2021 James McNaughton Felder
set -e
. ./iso.sh

"qemu-system-$(./target-triplet-to-arch.sh "$HOST")" -cdrom FelineOS.iso -boot d

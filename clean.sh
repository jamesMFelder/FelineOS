#!/bin/bash
# SPDX-License-Identifier: MIT
# Copyright (c) 2023 James McNaughton Felder
set -e
. ./config.sh

for PROJECT in $PROJECTS; do
  (cd "$PROJECT" && $MAKE clean)
done

rm -rf sysroot
rm -rf isodir
rm -f  FelineOS.iso
rm -f  kernel.sym
rm -rf docs/pdf
rm -rf docs/html

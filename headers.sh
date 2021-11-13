#!/bin/sh
# SPDX-License-Identifier: MIT
# Copyright (c) 2021 James McNaughton Felder
set -e
#shellcheck disable=SC1091
. ./config.sh

mkdir -p "$SYSROOT"

for PROJECT in $SYSTEM_HEADER_PROJECTS; do
  (cd "$PROJECT" && DESTDIR="$SYSROOT" $MAKE install-headers)
done

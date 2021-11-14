#!/bin/sh
# SPDX-License-Identifier: MIT
# Copyright (c) 2021 James McNaughton Felder
set -e
#shellcheck disable=SC1091
. ./headers.sh

for PROJECT in $PROJECTS; do
	#Allow parsing of the full build process
	echo "Making all in $PROJECT"
	(cd "$PROJECT" && DESTDIR="$SYSROOT" $MAKE install)
done

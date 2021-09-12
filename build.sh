#!/bin/sh
set -e
. ./headers.sh

for PROJECT in $PROJECTS; do
	#Allow parsing of the full build process
	echo "Making all in $PROJECT"
	(cd "$PROJECT" && DESTDIR="$SYSROOT" $MAKE install)
done

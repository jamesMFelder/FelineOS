#!/bin/bash
# SPDX-License-Identifier: MIT
# Copyright (c) 2023 James McNaughton Felder

# Make sure to get a full compilation
./clean.sh

# Create compile_commands.json from the build process
bear -- ./build.sh

# Fix it so that we don't jump to header files in the sysroot (which get overwritten with each rebuild)
sed -i '/sysroot\/\//c     "/home/james/src/FelineOS/system/libc/include", "-isystem", "/home/james/src/FelineOS/system/libc/include/c++", "-isystem", "/home/james/src/FelineOS/system/kernel/include", "-isystem", "/home/james/src/FelineOS/system/kernel/drivers/include", "-isystem", "/home/james/src/FelineOS/system/terminals/include", "-isystem", "/home/james/src/FelineOS/system/libFeline/include", "-isystem", "/home/james/src/FelineOS/system/libFeline/include/feline/c++",' compile_commands.json

# Define the correct macro for the current architecture
case "$(./default-host.sh)" in
	*arm*) ARCH_MACRO=__arm__ ;;
	*i686*) ARCH_MACRO=__i686__ ;;
	*) echo "Unknown architecture $TARGET_HOST, please add." ;;
esac
sed "/\(i686-elf\|arm-none-eabi\)-g../a\      \"-D${ARCH_MACRO}\"," compile_commands.json -i

#Create a symbol file for bochs
nm -Cg system/kernel/FelineOS.kernel | sed -n 's/^\([[:xdigit:]]\+\) [[:alpha:]] /0x\1 /p' > kernel.sym

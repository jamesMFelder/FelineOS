export SYSTEM_HEADER_PROJECTS="system/libc system/terminals system/kernel"
export PROJECTS="system/libc system/terminals system/kernel"

export MAKE=${MAKE:-make}
export HOST=${HOST:-$(./default-host.sh)}

export AR=${HOST}-ar
export AS=${HOST}-as
export CC=${HOST}-gcc
export CPP=${HOST}-g++

export NASM=nasm
export NASM_ARGS="-felf32 -w+all"

export PREFIX=/usr
export EXEC_PREFIX=$PREFIX
export BOOTDIR=/boot
export LIBDIR=$EXEC_PREFIX/lib
export INCLUDEDIR=$PREFIX/include
export CPP_INCLUDEDIR=$PREFIX/include/c++

export CFLAGS='-O0 -g -Wall -Wextra -Werror -pedantic -fstack-protector -mno-red-zone'
export CPPFLAGS="$CFLAGS -fno-rtti -fno-exceptions"

# Configure the cross-compiler to use the desired system root.
export SYSROOT="$PWD/sysroot"
export CC="$CC --sysroot=$SYSROOT"
export CPP="$CPP --sysroot=$SYSROOT"

# Work around that the -elf gcc targets doesn't have a system include directory
# because it was configured with --without-headers rather than --with-sysroot.
if echo "$HOST" | grep -Eq -- '-elf($|-)'; then
  export CC="$CC -isystem=$INCLUDEDIR"
  export CPP="$CPP -isystem=$INCLUDEDIR -isystem=$CPP_INCLUDEDIR"
fi

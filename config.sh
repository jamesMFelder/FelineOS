export SYSTEM_HEADER_PROJECTS="system/libc system/libFeline system/kernel"
export PROJECTS="system/libc system/libFeline system/kernel"

export MAKE=${MAKE:-make}
export HOST=${HOST:-$(./default-host.sh)}

#export COMPILER="gcc"
export COMPILER="llvm"

export GAR="${HOST}-ar"
export GAR_OPTS=""
export GAS="${HOST}-as"
export GAS_OPTS=""
export GCC="${HOST}-gcc"
export GCC_OPTS=""
export GPP="${HOST}-g++"
export GPP_OPTS=""

export LAR="llvm-ar"
export LAR_OPTS=""
export LAS="llvm-as"
export LAS_OPTS="-target ${HOST}"
export LCC="clang"
export LCC_OPTS="-target ${HOST} -march=i386 -Weverything -Wno-c++98-compat -Wno-c++98-compat-extra-semi -Wno-c++98-compat-pedantic -Wno-c++17-extensions -Wno-reserved-identifier -Wno-missing-variable-declarations -Wno-global-constructors -Wno-language-extension-token"
export LPP="clang++"
export LPP_OPTS="-target ${HOST} -march=i386 -Weverything -Wno-c++98-compat -Wno-c++98-compat-extra-semi -Wno-c++98-compat-pedantic -Wno-c++17-extensions -Wno-reserved-identifier -Wno-missing-variable-declarations -Wno-global-constructors -Wno-language-extension-token"

if [[ ${COMPILER} == "gcc" ]]; then
	export AR="${GAR} ${GAR_OPTS}"
	export AS="${GAS} ${GAS_OPTS}"
	export CC="${GCC} ${GCC_OPTS}"
	export CPP="${GPP} ${GPP_OPTS}"
elif [[ ${COMPILER} == "llvm" ]]; then
	export AR="${LAR} ${LAR_OPTS}"
	export AS="${LAS} ${LAS_OPTS}"
	export CC="${LCC} ${LCC_OPTS}"
	export CPP="${LPP} ${LPP_OPTS}"
else
	echo "Unknown compiler ${COMPILER}. Try setting to 'gcc' or 'llvm'." >&2
	exit 2
fi

export PREFIX=/usr
export EXEC_PREFIX=$PREFIX
export BOOTDIR=/boot
export LIBDIR=$EXEC_PREFIX/lib
export INCLUDEDIR=$PREFIX/include
export CPP_INCLUDEDIR=$PREFIX/include/c++

export CFLAGS='-O0 -g -Werror -Wall -Wextra -pedantic -fstack-protector -mno-red-zone'
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

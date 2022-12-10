export SYSTEM_HEADER_PROJECTS="system/libc system/libFeline system/kernel"
export PROJECTS="system/libc system/libFeline system/kernel"

export MAKE=${MAKE:-make}
export TARGET_HOST=${TARGET_HOST:-$(./default-host.sh)}
export ARCH_OPTS=${ARCH_OPTS:-$(./arch-opts.sh)}

export COMPILER="gcc"
#export COMPILER="llvm"

export GAR="${TARGET_HOST}-ar"
export GAR_OPTS=""
export GAS="${TARGET_HOST}-as"
export GAS_OPTS=""
export GCC="${TARGET_HOST}-gcc"
export GCC_OPTS="${ARCH_OPTS}"
export GPP="${TARGET_HOST}-g++"
export GPP_OPTS="${ARCH_OPTS}"

export LAR="llvm-ar"
export LAR_OPTS=""
export LAS="llvm-as"
export LAS_OPTS="-target ${TARGET_HOST}"
export LCC="clang"
export LCC_OPTS="-target ${TARGET_HOST} ${ARCH_OPTS} -Weverything -Wno-c++98-compat -Wno-c++98-compat-extra-semi -Wno-c++98-compat-pedantic -Wno-c++17-extensions -Wno-reserved-identifier -Wno-missing-variable-declarations -Wno-global-constructors -Wno-language-extension-token"
export LPP="clang++"
export LPP_OPTS="-target ${TARGET_HOST} ${ARCH_OPTS} -Weverything -Wno-c++98-compat -Wno-c++98-compat-extra-semi -Wno-c++98-compat-pedantic -Wno-c++17-extensions -Wno-reserved-identifier -Wno-missing-variable-declarations -Wno-global-constructors -Wno-language-extension-token"

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

export CFLAGS='-O0 -g -Werror -Wall -Wextra -fstack-protector -fno-omit-frame-pointer'
case "$TARGET_HOST" in
	*arm*) CFLAGS="$CFLAGS -mapcs-frame";;
esac
export CPPFLAGS="$CFLAGS -fno-rtti -fno-exceptions"

# Configure the cross-compiler to use the desired system root.
export SYSROOT="$PWD/sysroot"
export CC="$CC --sysroot=$SYSROOT"
export CPP="$CPP --sysroot=$SYSROOT"

# Work around that the our gcc targets doesn't have a system include directory
# because they were configured with --without-headers rather than --with-sysroot.
export CC="$CC -isystem $SYSROOT/$INCLUDEDIR"
export CPP="$CPP -isystem $SYSROOT/$INCLUDEDIR -isystem $SYSROOT/$CPP_INCLUDEDIR"

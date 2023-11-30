if (${CMAKE_SYSTEM_PROCESSOR} STREQUAL "i686")
	set(LIBC_ARCH_OBJS
		system/libc/arch/i386/syscall/syscall.S
		)
elseif(${CMAKE_SYSTEM_PROCESSOR} STREQUAL "arm")
	set(LIBC_ARCH_OBJS
		system/libc/arch/arm/syscall/syscall.S
		)
else()
	message(FATAL_ERROR "Unknown architecture " ${CMAKE_SYSTEM_PROCESSOR} ", expected i686 or arm")
endif()

set(LIBC_OBJS
	system/libc/ctype/iscntrl.cpp
	system/libc/ctype/isdigit.cpp

	system/libc/assert/assert.cpp

	system/libc/errno/errno.cpp

	system/libc/fcntl/open.cpp
	system/libc/fcntl/close.cpp

	system/libc/icxxabi/icxxabi.cpp

	system/libc/ssp/ssp.cpp

	system/libc/stdlib/abort.cpp
	system/libc/stdlib/abs.cpp
	system/libc/stdlib/malloc_free.cpp

	system/libc/stdio/io_common.cpp
	system/libc/stdio/printf.cpp
	system/libc/stdio/putchar.cpp
	system/libc/stdio/puts.cpp

	system/libc/string/memcpy.cpp
	system/libc/string/memset.cpp
	system/libc/string/memmove.cpp
	system/libc/string/strcmp.cpp
	system/libc/string/strcpy.cpp
	system/libc/string/strlen.cpp

	system/libc/unistd/read.cpp
	system/libc/unistd/write.cpp

	${LIBC_ARCH_OBJS}

	PARENT_SCOPE)

#include <stdio.h>

#if defined(__is_libk)
#include <kernel/tty.h>
#include <drivers/serial.h>
#endif //__is_libk

int putchar(int ic) {
#if defined(__is_libk)
	char c = (char) ic;
	terminal_putchar(c);
	put_serial(c);
#else //__is_libk
	// TODO: Implement stdio and the write system call.
#endif //__is_libk (else)
	return ic;
}

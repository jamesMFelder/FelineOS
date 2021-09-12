// SPDX-License-Identifier: MIT
// Copyright (c) 2021 James McNaughton Felder
#include <stdio.h>

#if defined(__is_libk)
#include <kernel/tty.h>
#include <drivers/serial.h>
#endif //__is_libk

int puts(const char* string) {
#if defined(__is_libk)
	terminal_writestring(string);
	terminal_putchar('\n');
	writestr_serial(string);
	put_serial('\n');
#else //__is_libk
	// TODO: Implement stdio and the write system call.
#endif //__is_libk (else)
	//TODO: check return values
	return 0;
}

#if defined(__is_libk)
int puts_no_nl(const char* string) {
	terminal_writestring(string);
	writestr_serial(string);
	return 0;
}
#endif //__is_libk

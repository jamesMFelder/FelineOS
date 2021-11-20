// SPDX-License-Identifier: MIT
// Copyright (c) 2021 James McNaughton Felder
#include <cstdio>
#include <cstdlib>

#if defined(__is_libk)
#include <kernel/log.h>
#include <kernel/backtrace.h>
#endif //__is_libk

__attribute__((__noreturn__))
void abort(void) {
#if defined(__is_libk)
	// TODO: Add proper kernel panic.
	kerror("abort()");
	backtrace();
#else //__is_libk
	// TODO: Abnormally terminate the process as if by SIGABRT.
	printf("abort()\n");
#endif //__is_libk (else)
	while (1) { }
	__builtin_unreachable();
}

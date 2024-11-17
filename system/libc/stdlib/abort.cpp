/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2023 James McNaughton Felder */
#include <cstdlib>

#if defined(__is_libk)
#include <kernel/backtrace.h>
#include <kernel/log.h>
#endif /* __is_libk */

__attribute__((__noreturn__)) void abort(void) {
#if defined(__is_libk)
	/* TODO: Add proper kernel panic. */
	kerror("std::abort()");
	backtrace();
#else  /* __is_libk */
	/* TODO: Abnormally terminate the process as if by SIGABRT. */
	printf("std::abort()\n");
#endif /* __is_libk (else) */
	while (true) {
	}
	__builtin_unreachable();
}

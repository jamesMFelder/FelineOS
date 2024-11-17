/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2023 James McNaughton Felder */

#include <bits/c_compat.h>
#include <feline/logger.h>

#if defined(__is_libk)
#include <kernel/backtrace.h>
#endif /* __is_libk */

C_LINKAGE __attribute__((__noreturn__)) void abort(void) {
	kCriticalNoAlloc() << "std::abort()";
#if defined(__is_libk)
	/* TODO: Add proper kernel panic. */
	backtrace();
#else /* __is_libk */
	/* TODO: Abnormally terminate the process as if by SIGABRT. */
#error "Userspace std::abort not supported yet!"
#endif /* __is_libk (else) */
	while (true) {
	}
	__builtin_unreachable();
}

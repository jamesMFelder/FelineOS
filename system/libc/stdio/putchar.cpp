/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2023 James McNaughton Felder */
#include <stdio.h>

#if defined(__is_libk)
#include <bits/io.h>
#endif /* __is_libk */

int putchar(int ic) {
#if defined(__is_libk)
	char c = static_cast<char>(ic);
	return __internal_putchar(c);
#else /* __is_libk */
	/* TODO: Implement stdio and the write system call. */
	return ic;
#endif /* __is_libk (else) */
}

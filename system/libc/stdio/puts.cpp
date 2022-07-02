/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2021 James McNaughton Felder */
#include <stdio.h>

#if defined(__is_libk)
#include <bits/io.h>
#endif /* __is_libk */

int puts(const char* string) {
#if defined(__is_libk)
	__internal_writeStr(string);
	__internal_putchar('\n');
#else /* __is_libk */
	/* TODO: Implement stdio and the write system call. */
#endif /* __is_libk (else) */
	/* TODO: check return values */
	return 0;
}

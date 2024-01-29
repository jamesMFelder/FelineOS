/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2023 James McNaughton Felder */
#ifndef _STDLIB_H
#define _STDLIB_H 1

#include <bits/c_compat.h>
#include <stddef.h>
#include <sys/cdefs.h>

C_LINKAGE __attribute__((__noreturn__)) void abort(void);

C_LINKAGE int abs(int j);
C_LINKAGE long labs(long j);
C_LINKAGE long long llabs(long long j);

C_LINKAGE void *malloc(size_t size);
C_LINKAGE void free(void *ptr);

#endif /* _STDLIB_H */

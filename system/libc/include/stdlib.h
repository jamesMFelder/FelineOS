/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2023 James McNaughton Felder */
#ifndef _STDLIB_H
#define _STDLIB_H 1

#include <bits/c_compat.h>
#include <sys/cdefs.h>
#include <stddef.h>

C_LINKAGE __attribute__((__noreturn__)) void abort(void);

C_LINKAGE int abs(int j);
C_LINKAGE long labs(long j);
C_LINKAGE long long llabs(long long j);

#endif /* _STDLIB_H */

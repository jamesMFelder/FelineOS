// SPDX-License-Identifier: MIT
// Copyright (c) 2021 James McNaughton Felder
#ifndef _STDIO_H
#define _STDIO_H 1

#include <bits/c_compat.h>
#include <sys/cdefs.h>

#define EOF (-1)

//Get __FelineOS_va_list (for vprintf) without dragging the whole <stdargs.h> header in.
#include <bits/va_list.h>

C_LINKAGE __attribute__ ((format (printf, 1, 2))) int printf(const char* __restrict, ...);
C_LINKAGE __attribute__ ((format (printf, 1, 0))) int vprintf(const char* __restrict, __FelineOS_va_list);
C_LINKAGE int putchar(int);
C_LINKAGE int puts(const char*);

#endif //_STDIO_H

// SPDX-License-Identifier: MIT
// Copyright (c) 2021 James McNaughton Felder
#ifndef _STDIO_H
#define _STDIO_H 1

#include <sys/cdefs.h>

#define EOF (-1)

//Get __FelineOS_va_list (for vprintf) without dragging the whole <stdargs.h> header in.
#include <bits/va_list.h>

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

__attribute__ ((format (printf, 1, 2)))
int printf(const char* __restrict, ...);
int vprintf(const char* __restrict, __FelineOS_va_list);
int putchar(int);
int puts(const char*);
//Not standard, but I want something to use in printf for %s
int puts_no_nl(const char*);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif //_STDIO_H

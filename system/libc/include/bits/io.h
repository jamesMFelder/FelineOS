// SPDX-License-Identifier: MIT
// Copyright (c) 2021 James McNaughton Felder
#ifndef _BITS_IO_H
#define _BITS_IO_H 1

#define EOF (-1)

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

#if defined(__is_libk)
	int __internal_putchar(char c);
	int __internal_writeStr(char const * const s);
#else //__is_libk
#error "Userspace stdio not implimented yet."
#endif //__is_libk (else)

#ifdef __cplusplus
}
#endif //__cplusplus

#endif //_BITS_IO_H

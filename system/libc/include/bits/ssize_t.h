/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2023 James McNaughton Felder */
#ifndef _BITS_SSIZE_T_H
#define _BITS_SSIZE_T_H 1

#ifdef __SIZE_TYPE__
// Hack from https://awesomekling.github.io/How-SerenityOS-declares-ssize_t/
#define unsigned signed
typedef __SIZE_TYPE__ ssize_t;
#undef unsigned
#else
#error How is __SIZE_TYPE__ not defined!
#endif

#endif /* _BITS_SSIZE_T_H */

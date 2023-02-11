/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2023 James McNaughton Felder */
#ifndef _VA_LIST_H
#define _VA_LIST_H 1

/* Only define va_list. */
/* I believe this is GCC/Clang specific, but I am probably tied to them in other worse ways. */
typedef __builtin_va_list __FelineOS_va_list;

#endif /* _VA_LIST_H */

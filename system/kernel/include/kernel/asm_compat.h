/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2021 James McNaughton Felder */
#ifndef _FELINE_COMPAT_H
#define _FELINE_COMPAT_H 1

/* Because we have no C in the kernel, calling it asm is more descriptive */
#ifdef __cplusplus
#define ASM extern "C"
#else
#define ASM
#endif

#endif /* _FELINE_COMPAT_H */

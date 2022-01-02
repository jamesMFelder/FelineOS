// SPDX-License-Identifier: MIT
// Copyright (c) 2021 James McNaughton Felder
#ifndef _IO_COMPAT_H
#define _IO_COMPAT_H 1

//Just do the #ifdef in one place
//After version 1.0 you are free to include this file rather than write these 5 lines yourself,
//	but before then I make no promises about this name remaining the same.
#ifdef __cplusplus
#define C_LINKAGE extern "C"
#else
#define C_LINKAGE
#endif

#endif // _IO_COMPAT_H

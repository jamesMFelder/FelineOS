// SPDX-License-Identifier: MIT
// Copyright (c) 2021 James McNaughton Felder
#ifndef _KERN_SYSCALL_H
#define _KERN_SYSCALL_H 1

#ifdef __cplusplus
extern "C"{
#endif
long syscall(long which, ...);
#ifdef __cplusplus
}
#endif

#endif //_KERN_SYSCALL_H

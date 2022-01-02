// SPDX-License-Identifier: MIT
// Copyright (c) 2021 James McNaughton Felder
#ifndef _KERN_SYSCALL_H
#define _KERN_SYSCALL_H 1

#include <bits/c_compat.h>

C_LINKAGE long syscall(long which, ...);

#endif //_KERN_SYSCALL_H

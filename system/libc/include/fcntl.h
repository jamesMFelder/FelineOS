/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2023 James McNaughton Felder */
#ifndef _FCNTL_H
#define _FCNTL_H 1

#include <bits/c_compat.h>
#include <sys/cdefs.h>

C_LINKAGE int open(char const *pathname, int flags);
C_LINKAGE int close(int fd);

#endif /* _FCNTL_H */

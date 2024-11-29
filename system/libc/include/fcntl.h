/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2023 James McNaughton Felder */
#ifndef _FCNTL_H
#define _FCNTL_H 1

#include <bits/c_compat.h>
#include <bits/pid_t.h> // IWYU pragma: export (type required by POSIX: https://pubs.opengroup.org/onlinepubs/9799919799/basedefs/fcntl.h.html)
#include <sys/cdefs.h>

C_LINKAGE int open(char const *pathname, int flags);
C_LINKAGE int close(int fd);

#endif /* _FCNTL_H */

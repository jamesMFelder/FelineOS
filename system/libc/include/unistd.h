/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2023 James McNaughton Felder */
#ifndef _UNISTD_H
#define _UNISTD_H 1

#include <bits/c_compat.h>
#include <bits/ssize_t.h>
#include <stddef.h>
#include <sys/cdefs.h>

C_LINKAGE ssize_t write(int fd, void const *buf, size_t len);
C_LINKAGE ssize_t read(int fd, void *buf, size_t len);

#endif /* _UNISTD_H */

/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2023 James McNaughton Felder */
#ifndef _STRING_H
#define _STRING_H 1

#include <bits/c_compat.h>
#include <stddef.h>
#include <sys/cdefs.h>

C_LINKAGE int memcmp(const void *, const void *, size_t);
C_LINKAGE void *memcpy(void *__restrict, const void *__restrict, size_t);
C_LINKAGE void *memmove(void *, const void *, size_t);
C_LINKAGE void *memset(void *, int, size_t);

C_LINKAGE size_t strlen(const char *);

C_LINKAGE char *strcpy(char *, const char *);
C_LINKAGE char *strncpy(char *, const char *, size_t);
C_LINKAGE size_t strlcpy(char *, const char *, size_t);

C_LINKAGE char *strcat(char *, const char *);
C_LINKAGE char *strncat(char *, const char *, size_t);
C_LINKAGE size_t strlcat(char *, const char *, size_t);

C_LINKAGE int strcmp(const char *, const char *);
C_LINKAGE int strncmp(const char *, const char *, size_t count);

C_LINKAGE char *strstr(const char *haystack, const char *needle);

#endif /* _STRING_H */

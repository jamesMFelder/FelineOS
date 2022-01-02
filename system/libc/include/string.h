// SPDX-License-Identifier: MIT
// Copyright (c) 2021 James McNaughton Felder
#ifndef _STRING_H
#define _STRING_H 1

#include <bits/c_compat.h>
#include <sys/cdefs.h>
#include <stddef.h>

C_LINKAGE int memcmp(const void*, const void*, size_t);
C_LINKAGE void* memcpy(void* __restrict, const void* __restrict, size_t);
C_LINKAGE void* memmove(void*, const void*, size_t);
C_LINKAGE void* memset(void*, int, size_t);

C_LINKAGE size_t strlen(const char*);

C_LINKAGE char* strcpy(char*, const char*);
C_LINKAGE char* strncpy(char*, const char*, size_t);
C_LINKAGE size_t strlcpy(char*, const char*, size_t);

C_LINKAGE char *strcat(char*, const char*);
C_LINKAGE char *strncat(char*, const char*, size_t);
C_LINKAGE size_t strlcat(char*, const char*, size_t);

#endif //_STRING_H

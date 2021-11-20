// SPDX-License-Identifier: MIT
// Copyright (c) 2021 James McNaughton Felder
#ifndef _STRING_H
#define _STRING_H 1

#include <sys/cdefs.h>

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

int memcmp(const void*, const void*, size_t);
void* memcpy(void* __restrict, const void* __restrict, size_t);
void* memmove(void*, const void*, size_t);
void* memset(void*, int, size_t);

size_t strlen(const char*);

char* strcpy(char*, const char*);
char* strncpy(char*, const char*, size_t);
size_t strlcpy(char*, const char*, size_t);

char *strcat(char*, const char*);
char *strncat(char*, const char*, size_t);
size_t strlcat(char*, const char*, size_t);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif //_STRING_H

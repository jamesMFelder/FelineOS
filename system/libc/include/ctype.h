// SPDX-License-Identifier: MIT
// Copyright (c) 2021 James McNaughton Felder
#ifndef _CTYPE_H
#define _CTYPE_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C"{
#endif
int isdigit(int c);
int iscntrl(int c);
#ifdef __cplusplus
}
#endif
#endif //_CTYPE_H

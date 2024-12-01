/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2023 James McNaughton Felder */
#ifndef _CTYPE_H
#define _CTYPE_H 1

#include <bits/c_compat.h>

C_LINKAGE int isdigit(int c);
C_LINKAGE int iscntrl(int c);
C_LINKAGE int isprint(int c);
C_LINKAGE int isspace(int c);

#endif /* _CTYPE_H */

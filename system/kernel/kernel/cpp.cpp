/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2023 James McNaughton Felder */

/* If assembly or undefined behavior creates a call to a pure virtual function */
/*	Given that this never should be called, we can't assume anything */
/*	so just don't do anything */
/* Seperate declaration and definition to quiet a warning. */
#include <bits/c_compat.h>
C_LINKAGE void __cxa_pure_virtual();
void __cxa_pure_virtual(){}

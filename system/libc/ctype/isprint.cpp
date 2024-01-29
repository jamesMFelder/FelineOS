/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2023 James McNaughton Felder */
#include <cctype>
#include <feline/bool_int.h>

int isprint(int c) {
	if (c >= ' ' && c <= '~') {
		return INT_TRUE;
	}
	return INT_FALSE;
}

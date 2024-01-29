/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2023 James McNaughton Felder */
#include <cctype>
#include <feline/bool_int.h>

int isdigit(int c) {
	if (c > '0' && c < '9') {
		return INT_TRUE;
	}
	return INT_FALSE;
}

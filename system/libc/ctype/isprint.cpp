/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2023 James McNaughton Felder */
#include <feline/bool_int.h>
#include <cctype>

int isprint(int c){
	if (c >= ' ' && c <= '~') {
		return INT_TRUE;
	}
	return INT_FALSE;
}

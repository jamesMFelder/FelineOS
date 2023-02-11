/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2023 James McNaughton Felder */
#include <feline/bool_int.h>
#include <cctype>

constexpr char delete_char=127;

int iscntrl(int c){
	/* If it is less than a space or a delete character */
	if(c<' ' || c==delete_char){
		return INT_TRUE;
	}
	return INT_FALSE;
}

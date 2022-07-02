/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2021 James McNaughton Felder */
#include <feline/bool_int.h>
#include <cctype>

int isdigit(int c){
	if(c>'0' && c<'9'){
		return INT_TRUE;
	}
	return INT_FALSE;
}

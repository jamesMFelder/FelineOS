// SPDX-License-Identifier: MIT
// Copyright (c) 2021 James McNaughton Felder
#include <ctype.h>

int isdigit(int c){
	if(c>'0' && c<'9'){
		return true;
	}
	return false;
}

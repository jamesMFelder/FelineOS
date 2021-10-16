// SPDX-License-Identifier: MIT
// Copyright (c) 2021 James McNaughton Felder
#include <ctype.h>

int iscntrl(int c){
	if(c<' ' || c=='\b'){
		return true;
	}
	return false;
}

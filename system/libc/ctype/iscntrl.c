// SPDX-License-Identifier: MIT
// Copyright (c) 2021 James McNaughton Felder
#include <ctype.h>
#include <stdbool.h>

int iscntrl(int c){
	if(c<' ' || c=='\b'){
		return true;
	}
	return false;
}

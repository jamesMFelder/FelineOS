// SPDX-License-Identifier: MIT
// Copyright (c) 2021 James McNaughton Felder
#include <cctype>
#include <cstdbool>

int iscntrl(int c){
	//If it is less than a space or a delete character
	if(c<' ' || c==127){
		return true;
	}
	return false;
}

/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2021 James McNaughton Felder */

#ifndef _FELINE_STR_H
#define _FELINE_STR_H 1

#include <feline/cpp_only.h>

int ntostr(unsigned long long const num, char str[9], unsigned const base);

template<class T>
int xtostr(const T num, char str[9], bool const upper=true){
	/* Do the conversion */
	int rval=ntostr(num, str, 16);
	/* Difference from the character after '9' to the first letter */
	char toLetter=upper?'A'-('9'+1):'a'-('9'+1);
	/* Loop through */
	for(int where=0;where<9;where++){
		/* If it isn't 0-9 */
		if(str[where]>'9'){
			/* Shift it to A-F or a-f */
			str[where]+=toLetter;
		}
	}
	return rval;
}

template<class T>
int otostr(const T num, char str[9]){
	/* Do the conversion */
	return ntostr(num, str, 8);
}

template<class T>
int itostr(const T num, char str[9]){
	/* Do the conversion */
	return ntostr(num, str, 10);
}

#endif /* _FELINE_STR_H */

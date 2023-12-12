/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2023 James McNaughton Felder */
#include <feline/str.h>

/* Convert a number to a string */
/* By assuming that base<=10, we force the caller to deal with higher bases with */
/* `if(c>'9') c+=39` for lowercase or `c+=7` for uppercase */
KString ntostr(unsigned long long const num, unsigned const base) {
	KString str;
	/* Make sure we have a valid base */
	if(base<2 || base>16){
		return str;
	}
	if (num == 0) {
		str.append('0');
		return str;
	}
	size_t str_len = 0;
	for (unsigned long long n = 1; n <= num; n *= base) {
		++str_len;
	}
	str.append('\0', str_len);
	/* Start with the 1s digits */
	unsigned long long todivby=1;
	/* Work right to left */
	/* Do this until it is in front of the string */
	/* Go backwards, and shift digits each time */
	for(size_t offset = str_len; offset > 0; --offset) {
		/* I have no idea how this can happen */
		str[offset-1]=static_cast<char>((num/todivby)%base)+'0';
		todivby *= base;
	}
	return str;
}

/* Convert a number to a string */
/* By assuming that base<=10, we force the caller to deal with higher bases with */
/* `if(c>'9') c+=39` for lowercase or `c+=7` for uppercase */
/* TODO: dynamic str length */
int ntostr(unsigned long long const num, char str[9], unsigned const base){
	/* Make sure we have a valid base */
	if(base<2 || base>16){
		return -1;
	}
	/* Start with the 1s digits */
	unsigned long long todivby=1;
	/* Work right to left */
	char *ptr=&str[8];
	/* Add a null so it is a valid string */
	*ptr='\0';
	/* Start by decrementing ptr because it just added a null */
	/* Do this until it is in front of the string */
	/* Go backwards, and shift digits each time */
	for(ptr--;ptr>=str;ptr--,todivby*=base){
		/* I have no idea how this can happen */
		*ptr=static_cast<char>((num/todivby)%base)+'0';
	}
	return 0;
}


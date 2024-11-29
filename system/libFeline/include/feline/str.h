/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2023 James McNaughton Felder */

#ifndef _FELINE_STR_H
#define _FELINE_STR_H 1

#include <feline/cpp_only.h>
#include <feline/kstring.h>

KString ntostr(unsigned long long const num, unsigned const base,
               unsigned const min_width);

template <class T>
KString xtostr(const T num, bool const upper = true,
               unsigned const min_width = 1) {
	/* Do the conversion */
	KString str = ntostr(num, 16, min_width);
	/* Difference from the character after '9' to the first letter */
	char toLetter = upper ? 'A' - ('9' + 1) : 'a' - ('9' + 1);
	/* Loop through */
	for (auto &c : str) {
		/* If it isn't 0-9 */
		if (c > '9') {
			/* Shift it to A-F or a-f */
			c += toLetter;
		}
	}
	return str;
}

template <class T> KString itostr(const T num, unsigned const min_width = 1) {
	return ntostr(num, 10, min_width);
}

template <class T> KString otostr(const T num, unsigned const min_width = 1) {
	return ntostr(num, 8, min_width);
}

template <class T> KString btostr(const T num, unsigned const min_width = 1) {
	return ntostr(num, 2, min_width);
}

int ntostr(unsigned long long const num, char str[17], unsigned const base);

template <class T>
int xtostr(const T num, char str[17], bool const upper = true) {
	/* Do the conversion */
	int rval = ntostr(num, str, 16);
	/* Difference from the character after '9' to the first letter */
	char toLetter = upper ? 'A' - ('9' + 1) : 'a' - ('9' + 1);
	/* Loop through */
	for (int where = 0; where < 20; where++) {
		/* If it isn't 0-9 */
		if (str[where] > '9') {
			/* Shift it to A-F or a-f */
			str[where] += toLetter;
		}
	}
	return rval;
}

template <class T> int btostr(const T num, char str[17]) {
	/* Do the conversion */
	return ntostr(num, str, 2);
}

template <class T> int otostr(const T num, char str[17]) {
	/* Do the conversion */
	return ntostr(num, str, 8);
}

template <class T> int itostr(const T num, char str[17]) {
	/* Do the conversion */
	return ntostr(num, str, 10);
}

#endif /* _FELINE_STR_H */

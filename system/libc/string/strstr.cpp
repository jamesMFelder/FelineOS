/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2024 James McNaughton Felder */

#include <cstring>

/* 7.26.5.1 The stateless search functions are generic functions. These
 * functions are generic in the qualification of the array to be
 * searched and will return a result pointer to an element with the same
 * qualification as the passed array. If the array to be searched is
 * const-qualified, the result pointer will be to a const-qualified
 * element. If the array to be searched is not const-qualified), the
 * result pointer will be to an unqualified element. The external
 * declarations of these generic functions have a concrete function type
 * that returns a pointer to an _unqualified_ element[…], and accepts a
 * pointer to a _ const-qualified _ array of the same type to search.
 * This signature supports all correct uses. If a macro definition of
 * any of these generic functions is suppressed to access an actual
 * function, the external declaration with the corresponding concrete
 * type is visible.) */
/* Because of the external declaration requirement, we must cast from const to
 * non-const. Hopefully the compiler still catches incorrect uses. */
char *strstr(const char *haystack, const char *needle) {
	if (needle == nullptr || *needle == '\0') {
		return const_cast<char *>(haystack);
	}
	if (haystack == nullptr) {
		return nullptr;
	}
	size_t needle_len = strlen(needle);
	while (*haystack != '\0') {
		if (strncmp(haystack, needle, needle_len) == 0) {
			return const_cast<char *>(haystack);
		}
	}
	return nullptr;
}

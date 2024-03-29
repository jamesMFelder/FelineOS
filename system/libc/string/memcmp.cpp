/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2023 James McNaughton Felder */
#include <cstring>

int memcmp(const void *aptr, const void *bptr, size_t size) {
	const unsigned char *a = static_cast<const unsigned char *>(aptr);
	const unsigned char *b = static_cast<const unsigned char *>(bptr);
	for (size_t i = 0; i < size; i++) {
		if (a[i] < b[i])
			return -1;
		else if (b[i] < a[i])
			return 1;
	}
	return 0;
}

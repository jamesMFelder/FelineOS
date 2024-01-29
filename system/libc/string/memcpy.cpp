/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2023 James McNaughton Felder */
#include <cstring>

void *memcpy(void *dstptr, const void *srcptr, size_t size) {
	unsigned char *dst = static_cast<unsigned char *>(dstptr);
	const unsigned char *src = static_cast<const unsigned char *>(srcptr);
	for (size_t i = 0; i < size; i++)
		dst[i] = src[i];
	return dstptr;
}

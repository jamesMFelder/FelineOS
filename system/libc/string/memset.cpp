/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2023 James McNaughton Felder */
#include <cstring>

void* memset(void* bufptr, int value, size_t size) {
	unsigned char* buf = static_cast<unsigned char*>(bufptr);
	for (size_t i = 0; i < size; i++)
		buf[i] = static_cast<unsigned char>(value);
	return bufptr;
}

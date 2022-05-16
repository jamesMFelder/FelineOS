// SPDX-License-Identifier: MIT
// Copyright (c) 2021 James McNaughton Felder
#include <cstring>

size_t strlen(const char* str) {
	size_t len = 0;
	while (str[len] != '\0')
		len++;
	return len;
}

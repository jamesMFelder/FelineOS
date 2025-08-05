/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2025 James McNaughton Felder */
#include <cstring>

size_t strnlen(const char *str, size_t maxlen) {
	size_t len = 0;
	while (len < maxlen && str[len] != '\0')
		len++;
	return len;
}

/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2023 James McNaughton Felder */
#include <cstring>
#include <limits>

int strcmp(const char* lhs, const char* rhs) {
	for (size_t i = 0; i < std::numeric_limits<size_t>::max(); i++) {
		if (lhs[i] < rhs[i])
			return -1;
		else if (rhs[i] < lhs[i])
			return 1;
		else if (rhs[i]=='\0')
			return 0;
	}
	return 0;
}

int strncmp(const char* lhs, const char* rhs, size_t size) {
	for (size_t i = 0; i < size; i++) {
		if (lhs[i] < rhs[i])
			return -1;
		else if (rhs[i] < lhs[i])
			return 1;
	}
	return 0;
}

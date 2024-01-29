/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2024 James McNaughton Felder */

#include <cstdlib>
#include <feline/kallocator.h>

void *get_memory(size_t min_len) {
	void *addr;
	if (!(addr = malloc(min_len))) {
		std::abort();
	}
	return addr;
}

void return_memory(void *addr, size_t) {
	free(addr);
}

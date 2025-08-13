/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2024 James McNaughton Felder */

#ifndef _FELINE_KALLOCATOR_H
#define _FELINE_KALLOCATOR_H 1

#include <cstddef>
#include <cstdlib>
#include <limits>

void *get_memory(size_t min_len);
void return_memory(void *addr, size_t len);

template <typename T> struct KGeneralAllocator {
		typedef T value_type;

		[[nodiscard]] T *allocate(std::size_t count) {
			if (std::numeric_limits<size_t>::max() / sizeof(value_type) <
			    count) {
				std::abort();
			}
			return static_cast<T *>(get_memory(count * sizeof(value_type)));
		}

		void deallocate(T *addr, std::size_t count) {
			return return_memory(addr, count * sizeof(value_type));
		}
};

#endif // _FELINE_KALLOCATOR_H

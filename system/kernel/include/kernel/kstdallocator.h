/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2023 James McNaughton Felder */
#ifndef _KERN_KSTDALLOCATOR_H
#define _KERN_KSTDALLOCATOR_H 1

#include <cstdlib>
#include <limits>
#include <memory>

template <typename T>
struct KGeneralAllocator {
	using value_type = T;

	KGeneralAllocator() = default;
	template <class U>
		KGeneralAllocator(const KGeneralAllocator<U>&) {}

	[[nodiscard]] T* allocate(std::size_t n) {
		if (n <= std::numeric_limits<std::size_t>::max() / sizeof(T)) {
			if (auto ptr = malloc(n * sizeof(T))) {
				return static_cast<T*>(ptr);
			}
			else {
				std::abort();
			}
		}
		else {
			std::abort();
		}
	}
	void deallocate(T* ptr, std::size_t n[[maybe_unused]]) {
		free(ptr);
	}
};

template <typename T, typename U>
inline bool operator == (const KGeneralAllocator<T>&, const KGeneralAllocator<U>&) {
	return true;
}

template <typename T, typename U>
inline bool operator != (const KGeneralAllocator<T>& a, const KGeneralAllocator<U>& b) {
	return !(a == b);
}

#endif /* _KERN_KSTDALLOCATOR_H */

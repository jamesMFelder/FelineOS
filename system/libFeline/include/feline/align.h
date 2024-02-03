/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2023 James McNaughton Felder */
#ifndef FELINE_ALIGN_H
#define FELINE_ALIGN_H 1

#include <cstddef>
#include <cstdint>
#include <feline/nonzero.h>

inline uintptr_t round_up_to_alignment(uintptr_t addr, NonZero<size_t> align) {
	if (addr % align == 0) {
		return addr;
	} else {
		return addr + align - (addr % align);
	}
}

inline uintptr_t round_up_to_alignment(uintptr_t addr, size_t align) {
	return round_up_to_alignment(addr, NonZero<size_t>(align));
}

inline void const *round_up_to_alignment(void const *addr, size_t align) {
	return reinterpret_cast<void const *>(
		round_up_to_alignment(reinterpret_cast<uintptr_t>(addr), align));
}

template <typename T>
	requires requires { !std::is_void_v<T>; }
inline T const *round_up_to_alignment(T const *addr) {
	return reinterpret_cast<T *>(
		round_up_to_alignment(reinterpret_cast<uintptr_t>(addr), alignof(T)));
}

#endif /* FELINE_ALIGN_H */

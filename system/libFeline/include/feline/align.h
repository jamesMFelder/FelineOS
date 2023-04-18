/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2023 James McNaughton Felder */
#ifndef FELINE_ALIGN_H
#define FELINE_ALIGN_H 1

#include <cstddef>
#include <cstdint>

inline void const* round_up_to_alignment(void const* addr, size_t align) {
	return static_cast<unsigned char const*>(addr)+align-(reinterpret_cast<uintptr_t>(addr)%align);
}

template<typename T>
inline T const* round_up_to_alignment(T const* addr) {
	return round_up_to_alignment(addr, alignof(T));
}

#endif /* FELINE_ALIGN_H */

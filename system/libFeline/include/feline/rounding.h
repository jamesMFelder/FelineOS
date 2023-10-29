/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2023 James McNaughton Felder */
#ifndef _FELINE_ROUNDING_H
#define _FELINE_ROUNDING_H 1

#include <feline/cpp_only.h>
#include <type_traits>

template <typename T>
requires std::is_integral_v<T> && std::is_unsigned_v<T>
T round_to_multiple_of(T value, T multiple) {
	if (multiple == 0) {
		return value;
	}

	T remainder = value % multiple;
	if (remainder == 0) {
		return value;
	}
	return value + multiple - remainder;
}

#endif /* _FELINE_ROUNDING_H */

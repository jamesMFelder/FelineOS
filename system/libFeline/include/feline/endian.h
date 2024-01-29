/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2023 James McNaughton Felder */
#ifndef _FELINE_ENDIAN_H
#define _FELINE_ENDIAN_H 1

/*
 * Some wrappers for writing code that reads structures
 *   with a specific endianness, but runs on computers
 *   with different endiannesses.
 */

#include <feline/reverse_endian.h>

template <EndianReverseable T> class native_endian {
	public:
		constexpr inline operator T() const { return value; };

	private:
		T value;
};

template <EndianReverseable T> class nonnative_endian {
	public:
		constexpr inline operator T() const { return reverse_endian(value); };

	private:
		T value;
};

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
template <EndianReverseable T> using little_endian = native_endian<T>;
template <EndianReverseable T> using big_endian = nonnative_endian<T>;
#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
template <typename T> using big_endian = native_endian<T>;
template <typename T> using little_endian = reverse_endian<T>;
#else
#error "Mixed-endian architectures are not supported!"
#endif

#endif /* _FELINE_ENDIAN_H */

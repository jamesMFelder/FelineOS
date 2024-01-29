/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2023 James McNaughton Felder */
#ifndef _FELINE_REVERSE_ENDIAN_H
#define _FELINE_REVERSE_ENDIAN_H 1

#include <concepts>
#include <cstdint>

template <typename T> constexpr T reverse_endian(T value);

inline uint16_t reverse_endian(uint16_t value) {
	uint16_t reversed = 0;
	reversed |= (value & 0xff00) >> 8;
	reversed |= (value & 0x00ff) << 8;
	return reversed;
}

inline uint32_t reverse_endian(uint32_t value) {
	uint32_t reversed = 0;
	reversed |= (value & 0xff000000) >> 24;
	reversed |= (value & 0x00ff0000) >> 8;
	reversed |= (value & 0x0000ff00) << 8;
	reversed |= (value & 0x000000ff) << 24;
	return reversed;
}

inline uint64_t reverse_endian(uint64_t value) {
	uint64_t reversed = 0;
	reversed |= (value & 0xff00000000000000) >> 56;
	reversed |= (value & 0x00ff000000000000) >> 40;
	reversed |= (value & 0x0000ff0000000000) >> 24;
	reversed |= (value & 0x000000ff00000000) >> 8;
	reversed |= (value & 0x00000000ff000000) << 8;
	reversed |= (value & 0x0000000000ff0000) << 24;
	reversed |= (value & 0x000000000000ff00) << 40;
	reversed |= (value & 0x00000000000000ff) << 56;
	return reversed;
}

template <typename T>
concept EndianReverseable = requires(T a) {
	{ reverse_endian(a) } -> std::same_as<T>;
};

#endif /* _FELINE_REVERSE_ENDIAN_H */

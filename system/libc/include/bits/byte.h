/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2023 James McNaughton Felder */
#ifndef _BITS_BYTE_H
#define _BITS_BYTE_H 1

#include <type_traits>

// From https://en.cppreference.com/w/cpp/types/byte

namespace std {
enum class byte : unsigned char {};

template <class IntegerType>
	requires is_integral_v<IntegerType>
constexpr IntegerType to_integer(byte b) {
	return IntegerType(b);
}

template <class IntegerType>
	requires is_integral_v<IntegerType>
constexpr byte &operator<<=(byte &b, IntegerType shift) noexcept {
	return b = b << shift;
}

template <class IntegerType>
	requires is_integral_v<IntegerType>
constexpr byte &operator>>=(byte &b, IntegerType shift) noexcept {
	return b = b >> shift;
}

template <class IntegerType>
	requires is_integral_v<IntegerType>
constexpr byte operator<<(byte b, IntegerType shift) noexcept {
	return b << shift;
}

template <class IntegerType>
	requires is_integral_v<IntegerType>
constexpr byte operator>>(byte b, IntegerType shift) noexcept {
	return b >> shift;
}

constexpr byte operator|(byte l, byte r) noexcept {
	return byte(static_cast<unsigned int>(l) | static_cast<unsigned int>(r));
}

constexpr byte operator&(byte l, byte r) noexcept {
	return byte(static_cast<unsigned int>(l) & static_cast<unsigned int>(r));
}

constexpr byte operator^(byte l, byte r) noexcept {
	return byte(static_cast<unsigned int>(l) ^ static_cast<unsigned int>(r));
}

constexpr byte &operator|=(byte &l, byte r) noexcept { return l = l | r; }

constexpr byte &operator&=(byte &l, byte r) noexcept { return l = l & r; }

constexpr byte &operator^=(byte &l, byte r) noexcept { return l = l ^ r; }

constexpr byte operator~(byte b) noexcept {
	return byte(~static_cast<unsigned int>(b));
}

} // namespace std

#endif // _BITS_BYTE_H

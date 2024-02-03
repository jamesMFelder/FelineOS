/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2024 James McNaughton Felder */
#ifndef FELINE_NONZERO_H
#define FELINE_NONZERO_H 1

#include <cstdlib>
#include <feline/kernel_exceptions.h>
#include <type_traits>

template <typename T>
	requires(std::is_unsigned_v<T>)
class NonZero {
	public:
		constexpr NonZero(T const val) {
			if (val == 0) {
				report_fatal_error("Value should not be 0!");
			}
			value = val;
		}
		constexpr NonZero(NonZero const &other) { value = other; }
		constexpr operator T() const { return value; }
		constexpr NonZero &operator=(T const &other) {
			if (other == 0) {
				report_fatal_error("Value should not be 0!");
			}
			value = other;
			return *this;
		}
		constexpr NonZero &operator=(NonZero const &other) {
			value = other;
			return this;
		}

	private:
		T value;
};

#endif /* FELINE_NONZERO_H */

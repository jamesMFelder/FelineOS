/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2024 James McNaughton Felder */

#ifndef FELINE_TESTS_H
#define FELINE_TESTS_H 1

#include "kstring.h"
#include "logger.h"
#include "settings.h"
#include <cstdlib>
#include <initializer_list>

#ifdef LIBFELINE_ONLY
#include <iostream>

inline void initialize_loggers() {
	Settings::Logging::critical.initialize(
		[](char const *str, size_t len) { std::cerr.write(str, len); });
	Settings::Logging::error.initialize(
		[](char const *str, size_t len) { std::cerr.write(str, len); });
	Settings::Logging::warning.initialize(
		[](char const *str, size_t len) { std::cerr.write(str, len); });
	Settings::Logging::log.initialize(
		[](char const *str, size_t len) { std::clog.write(str, len); });
	Settings::Logging::debug.initialize(
		[](char const *str, size_t len) { std::clog.write(str, len); });
};

#endif // LIBFELINE_ONLY

template <typename T, typename Allocator>
inline kout &operator<<(kout &output, std::vector<T, Allocator> vec) {
	output << '[';
	if (!empty(vec)) {
		output << *begin(vec);
		std::for_each(begin(vec) + 1, end(vec),
		              [&output](const T &value) { output << ", " << value; });
	}
	output << ']';
	return output;
}

template <typename T>
kout &operator<<(kout &output, std::initializer_list<T> list) {
	output << '[';
	if (!empty(list)) {
		output << *list.begin();
		std::for_each(list.begin() + 1, list.end(),
		              [&output](const T value) { output << ", " << value; });
	}
	output << ']';
	return output;
}

#define OUTPUT_PTR(type)                                                       \
	inline kout &operator<<(kout &output, type const *pointer) {               \
		return output << ptr(pointer);                                         \
	}

OUTPUT_PTR(uint8_t)
OUTPUT_PTR(uint16_t)
OUTPUT_PTR(uint32_t)
OUTPUT_PTR(uint64_t)

#undef OUTPUT_PTR

template <typename T, typename Allocator>
bool operator==(std::vector<T, Allocator> vec, std::initializer_list<T> list) {
	return std::equal(vec.begin(), vec.end(), list.begin(), list.end());
}

template <typename T, typename Allocator>
bool operator!=(std::vector<T, Allocator> vec, std::initializer_list<T> list) {
	return !(vec == list);
}

template <typename A, typename B>
void require_eq(A a, KStringView a_str, B b, KStringView b_str) {
	if (a != b) {
		kCritical() << "FAILED " << a_str << "==" << b_str << " (got " << a
					<< "==" << b << ")";
		exit(EXIT_FAILURE);
	}
}

#define REQUIRE_EQ(a, b) require_eq(a, #a, b, #b);

#endif /* FELINE_TESTS_H */

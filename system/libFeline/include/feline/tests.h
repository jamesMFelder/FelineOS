/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2024 James McNaughton Felder */

#ifndef FELINE_TESTS_H
#define FELINE_TESTS_H 1

#ifdef LIBFELINE_ONLY
#include "settings.h"
#include <cstdlib>
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

template <typename T, typename Allocator>
std::ostream &operator<<(std::ostream &output, std::vector<T, Allocator> vec) {
	output << '[';
	if (!empty(vec)) {
		output << *vec.begin();
		std::for_each(vec.begin() + 1, vec.end(),
		              [&output](const T value) { output << ", " << value; });
	}
	output << ']';
	return output;
}

template <typename T>
std::ostream &operator<<(std::ostream &output, std::initializer_list<T> list) {
	output << '[';
	if (!empty(list)) {
		output << *list.begin();
		std::for_each(list.begin() + 1, list.end(),
		              [&output](const T value) { output << ", " << value; });
	}
	output << ']';
	return output;
}

template <typename T, typename Allocator>
bool operator==(std::vector<T, Allocator> vec, std::initializer_list<T> list) {
	return std::equal(vec.begin(), vec.end(), list.begin(), list.end());
}

template <typename T, typename Allocator>
bool operator!=(std::vector<T, Allocator> vec, std::initializer_list<T> list) {
	return !(vec == list);
}

template <typename A, typename B>
void require_eq(A a, std::string_view a_str, B b, std::string_view b_str) {
	if (a != b) {
		std::clog << "FAILED " << a_str << "==" << b_str << " (got " << a
				  << "==" << b << ")" << std::endl;
		exit(EXIT_FAILURE);
	}
}

#define REQUIRE_EQ(a, b) require_eq(a, #a, b, #b);

#else
#error "Freestanding output for tests not figured out yet!"
#endif

#endif /* FELINE_TESTS_H */

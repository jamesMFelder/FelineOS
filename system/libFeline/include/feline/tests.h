/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2024 James McNaughton Felder */

#ifndef FELINE_TESTS_H
#define FELINE_TESTS_H 1

#include <feline/kallocator.h>
#include <feline/kstring.h>
#include <feline/kvector.h>
#include <feline/logger.h>
#include <initializer_list>

#ifdef LIBFELINE_ONLY
#include <feline/settings.h>
#include <iostream>
#include <vector>

#define ADD_TEST(TESTNAME) int main()

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

#else // LIBFELINE_ONLY

#define DO_CONCAT(a, b, c) a##b##c
#define CONCAT(a, b, c) DO_CONCAT(a, b, c)
#define ENABLE_FUNCNAME(name) CONCAT(enable_, name, _test)
#define RUN_FUNCNAME(name) CONCAT(run_, name, _test)
#define ADD_TEST(TESTNAME)                                                     \
	int RUN_FUNCNAME(TESTNAME)();                                              \
	__attribute__((constructor)) void ENABLE_FUNCNAME(TESTNAME)() {            \
		test_functions.append(                                                 \
			test_func{.name = #TESTNAME, .func = &RUN_FUNCNAME(TESTNAME)});    \
	}                                                                          \
	int RUN_FUNCNAME(TESTNAME)()

inline void initialize_loggers() {}

struct test_func {
		KStringView name;
		int (*func)();
};

extern KVector<test_func, KGeneralAllocator<test_func>> test_functions;

#endif // LIBFELINE_ONLY (else)

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

inline bool require(bool a, KStringView a_str) {
	if (!a) {
		kCritical() << "FAILED " << a_str;
		return false;
	}
	return true;
}

template <typename A, typename B>
bool require_eq(A a, KStringView a_str, B b, KStringView b_str) {
	if (a != b) {
		kCritical() << "FAILED " << a_str << "==" << b_str << " (got " << a
					<< "==" << b << ")";
		return false;
	}
	return true;
}

#define REQUIRE(a)                                                             \
	if (!require(a, #a)) {                                                     \
		return 1;                                                              \
	}
#define REQUIRE_NOT(a)                                                         \
	if (!require(!a, "!" #a)) {                                                \
		return 1;                                                              \
	}
#define REQUIRE_EQ(a, b)                                                       \
	if (!require_eq(a, #a, b, #b)) {                                           \
		return 1;                                                              \
	}

#endif /* FELINE_TESTS_H */

/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2023 James McNaughton Felder */
#ifndef _FELINE_SHORTCUTS_H
#define _FELINE_SHORTCUTS_H 1

#include <cstddef>
#include <feline/cpp_only.h>

template <typename T>
concept container = requires(T c) {
	{ c.size() };
	{ c.data() };
	{ c[0] };
	typename T::value_type;
	typename T::iterator;
};

template <typename T>
concept has_begin_end =
	requires(T container) { container.begin() && container.end(); };

template <container T>
	requires has_begin_end<T>
auto begin(T &c) -> decltype(c.begin()) {
	return c.begin();
}

template <container T>
	requires has_begin_end<T>
auto end(T &c) -> decltype(c.end()) {
	return c.end();
}

template <container T>
	requires(!has_begin_end<T>)
auto begin(T &c) -> decltype(c.data()) {
	return c.data();
}

template <container T>
	requires(!has_begin_end<T>)
auto end(T &c) -> decltype(c.data() + c.size()) {
	return c.data() + c.size();
}

template <typename T, size_t N> T *begin(T (&array)[N]) { return array; }

template <typename T, size_t N> T *end(T (&array)[N]) { return array + N; }

#endif /* _FELINE_SHORTCUTS_H */

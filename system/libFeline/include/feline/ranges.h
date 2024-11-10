/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2023 James McNaughton Felder */
#ifndef FELINE_RANGES_H
#define FELINE_RANGES_H 1

#include <concepts>
#include <feline/cpp_only.h>

/* A basic range type. */
template <typename T>
	requires std::totally_ordered<T>
struct range {
		T start, end;
};

template <typename T> bool overlap(range<T> a, range<T> b) {
	return max(a.start, a.end) >= b.start && min(a.start, a.end) <= b.end;
}

#endif // FELINE_RANGES_H

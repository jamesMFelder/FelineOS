/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2023 James McNaughton Felder */
#ifndef FELINE_MINMAX_H
#define FELINE_MINMAX_H 1

#include <feline/cpp_only.h>

template <class T> inline T min(T a, T b) {
	if (a > b) {
		return b;
	} else {
		return a;
	}
}

template <class T> inline T max(T a, T b) {
	if (a > b) {
		return a;
	} else {
		return b;
	}
}

#endif /* FELINE_MINMAX_H */

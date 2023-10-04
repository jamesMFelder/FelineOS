/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2023 James McNaughton Felder */
#ifndef FELINE_KSTRING_H
#define FELINE_KSTRING_H 1

#include <cstddef>

/* KStringView: a no allocation string suitable for constant data */
class KStringView {
	public:
		KStringView() : data(nullptr), len(0) {}
		KStringView(char const *data, size_t len) : data(data), len(len) {}
		KStringView(const KStringView &) = default;
		KStringView(KStringView &&) = default;
		~KStringView();

		KStringView &operator=(const KStringView &) = default;
		KStringView &operator=(KStringView &&) = default;

		char const* get() const;
		size_t length() const;

		operator char const*() const;

	private:
		char const *data;
		size_t len;
};

inline KStringView const operator""_kstr(char const *str, size_t len) {
	return KStringView(str, len);
};

#endif /* FELINE_KSTRING_H */

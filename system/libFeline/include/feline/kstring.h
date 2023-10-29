/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2023 James McNaughton Felder */

#ifndef _FELINE_KSTRING_H
#define _FELINE_KSTRING_H 1

#include <cstring>
#include <cstddef>
#include <cstdlib>
#include <feline/cpp_only.h>
#include <kernel/kstdallocator.h>
#include <feline/kvector.h>

class KStringView {
	public:
		using value_type = char;
		using const_pointer = value_type const *;
		using pointer = const_pointer;
		using const_reference = value_type const &;
		using reference = const_reference;
		using const_iterator = value_type const *;
		using iterator = const_pointer;

		KStringView(char const *characters, size_t len) : characters(characters), len(len) {}
		KStringView(char const *characters) : characters(characters), len(strlen(characters)) {}
		KStringView(KStringView &other) { *this = other; }
		KStringView &operator=(KStringView &other) {characters = other.data(); len = other.length(); return *this; }

		const_reference get(size_t index) const {
			if (index >= len) {
				std::abort();
			}
			return characters[index];
		};
		const_reference operator[](size_t index) const {
			return get(index);
		}

		char const *data() const { return characters; };
		size_t length() const { return len; };
		size_t size() const { return len; };

	private:
		char const *characters;
		size_t len;
};

inline KStringView operator""_kstr(char const *characters, size_t len) { return KStringView(characters, len); };

using KString = KVector<char, KGeneralAllocator<char>>;
using KConstString = KVector<char const, KGeneralAllocator<char>>;

inline KConstString operator""_kstr_vec(char const *characters, size_t len) { return KConstString(characters, len); };

#endif /* _FELINE_KSTRING_H */

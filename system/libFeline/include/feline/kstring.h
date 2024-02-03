/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2023 James McNaughton Felder */

#ifndef _FELINE_KSTRING_H
#define _FELINE_KSTRING_H 1

#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <feline/cpp_only.h>
#include <feline/kallocator.h>
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

		constexpr KStringView() : characters(nullptr), len(0) {}
		constexpr KStringView(char const *characters, size_t len)
			: characters(characters), len(len) {}
		KStringView(char const *characters)
			: characters(characters), len(strlen(characters)) {}
		constexpr KStringView(KStringView const &other) { *this = other; }
		constexpr KStringView &operator=(KStringView const &other) {
			characters = other.data();
			len = other.length();
			return *this;
		}

		constexpr const_reference get(size_t index) const {
			if (index >= len) {
				std::abort();
			}
			return characters[index];
		};
		constexpr const_reference operator[](size_t index) const {
			return get(index);
		}

		constexpr char const *data() const { return characters; };
		constexpr size_t length() const { return len; };
		constexpr size_t size() const { return len; };

	private:
		char const *characters;
		size_t len;
};

inline consteval KStringView operator""_kstr(char const *characters,
                                             size_t len) {
	return KStringView(characters, len);
};

using KString = KVector<char, KGeneralAllocator<char>>;
using KConstString = KVector<char const, KGeneralAllocator<char>>;

inline consteval KConstString operator""_kstr_vec(char const *characters,
                                                  size_t len) {
	return KConstString(characters, len);
};

#endif /* _FELINE_KSTRING_H */

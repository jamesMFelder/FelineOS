/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2023 James McNaughton Felder */

#ifndef _FELINE_KVECTOR_H
#define _FELINE_KVECTOR_H 1

#include <algorithm>
#include <cstddef>
#include <cstdlib>
#include <feline/cpp_only.h>
#include <feline/shortcuts.h>
#include <memory>
#include <type_traits>

void check_index(size_t index, size_t max);

template <typename T, typename Allocator> class KVector {
	public:
		using value_type = T;
		using pointer = T *;
		using const_pointer = T const *;
		using reference = T &;
		using const_reference = T const &;
		using iterator = T *;
		using const_iterator = T const *;

		constexpr KVector()
			: items(nullptr), num_items(0), m_capacity(0), a() {}
		constexpr KVector(pointer items, size_t size)
			: items(items), num_items(size), m_capacity(size), a() {}
		constexpr KVector(pointer items, size_t size)
			requires(!std::is_const_v<T>)
			: a() {
			this->items = a.allocate(size);
			this->num_items = size;
			this->capacity = size;
			std::uninitialized_copy_n(items, size, this->items);
		}
		constexpr KVector(KVector &&other) : KVector() { swap(*this, other); }
		constexpr KVector(KVector const &other)
			requires(std::is_const_v<T>)
			: items(other.items()), num_items(other.size()),
			  m_capacity(other.size()), a() {}
		constexpr KVector(KVector const &other)
			requires(!std::is_const_v<T>)
			: num_items(other.size()), m_capacity(other.size()), a() {
			this->items = a.allocate(m_capacity);
			std::uninitialized_copy(begin(other), end(other), items);
		}

		constexpr ~KVector()
			requires(std::is_const_v<T>)
		{}
		constexpr ~KVector()
			requires(!std::is_const_v<T>)
		{
			a.deallocate(items, m_capacity);
		}

		constexpr KVector &operator=(KVector &&other) {
			swap(*this, other);
			return *this;
		}

		constexpr reference get(size_t index) {
			check_index(index, num_items);
			return items[index];
		};
		constexpr const_reference get(size_t index) const {
			check_index(index, num_items);
			return items[index];
		};

		constexpr reference operator[](size_t index) {
			check_index(index, num_items);
			return items[index];
		};
		constexpr T const &operator[](size_t index) const {
			check_index(index, num_items);
			return items[index];
		};

		constexpr void set(reference item, size_t index) {
			check_index(index, num_items);
			items[index] = item;
		};

		constexpr size_t capacity() const { return m_capacity; }
		constexpr size_t size() const { return num_items; }
		constexpr pointer data() { return items; }
		constexpr const_pointer data() const { return items; }

		void reserve(size_t num) {
			if (num <= m_capacity) {
				return;
			}
			auto new_items = a.allocate(num);
			items &&std::move(begin(*this), end(*this), new_items);
			a.deallocate(items, m_capacity);
			m_capacity = num;
			items = new_items;
		}

		void push_back(value_type item) { append(item, 1); }

		void append(value_type item, size_t count = 1) {
			reserve(num_items + count);
			for (size_t i = 0; i < count; ++i) {
				new (&items[num_items + i]) T(std::move(item));
			}
			num_items += count;
		}
		template <size_t N> void append(T (&other)[N]) {
			return append(KVector(other, N));
		}
		void append(const_pointer other, size_t len) {
			append(KVector<T const, Allocator>(other, len));
		}
		void append(KVector<T, Allocator> other) {
			if (m_capacity < (num_items + other.size()) || !items) {
				reserve(num_items + other.size());
			}
			std::uninitialized_copy(begin(other), end(other),
			                        &items[num_items]);
			num_items += other.size();
		}
		void append(KVector<T const, Allocator> other)
			requires(std::is_same_v<std::remove_const_t<T>, T>)
		{
			reserve(num_items + other.size());
			std::uninitialized_copy(begin(other), end(other),
			                        &items[num_items]);
			num_items += other.size();
		}

		void append(const_iterator first, const_iterator last) {
			reserve(num_items + std::distance(first, last));
			std::uninitialized_copy(first, last, &items[num_items]);
			num_items += std::distance(first, last);
		}

		template <size_t N> void operator+=(T (&other)[N]) {
			return append(other);
		}
		void operator+=(KVector<T, Allocator> other) { return append(other); }
		void operator+=(KVector<T const, Allocator> other)
			requires(std::is_same_v<std::remove_const_t<T>, T>)
		{
			return append(other);
		}

		constexpr iterator erase(iterator pos) {
			if (pos == end(*this)) {
				return pos;
			}
			std::move(pos + 1, end(*this), pos);
			--num_items;
			return pos;
		}
		constexpr iterator erase(iterator first, iterator last) {
			if (first == last) {
				return last;
			}
			std::move(last, end(*this), first);
			num_items -= std::distance(first, last);
			return last;
		}

		constexpr void clear() {
			if constexpr (requires { items->~value_type; }) {
				for (auto *elem : this) {
					elem->~value_type();
				}
			}
			num_items = 0;
		}
		friend void swap(KVector &first, KVector &second) {
			using std::swap;
			swap(first.items, second.items);
			swap(first.num_items, second.num_items);
			swap(first.m_capacity, second.m_capacity);
			swap(first.a, second.a);
		}

	private:
		pointer items;
		size_t num_items;
		size_t m_capacity;
		Allocator a;
};

#endif /* _FELINE_KVECTOR_H */

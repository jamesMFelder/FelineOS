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
			requires(!std::is_const_v<value_type>)
			: a() {
			this->items = a.allocate(size);
			this->num_items = size;
			this->m_capacity = size;
			std::uninitialized_copy_n(items, size, this->items);
		}
		constexpr KVector(KVector &&other) : KVector() { swap(*this, other); }
		constexpr KVector(KVector const &other)
			requires(std::is_const_v<value_type>)
			: items(other.items()), num_items(other.size()),
			  m_capacity(other.size()), a() {}
		constexpr KVector(KVector const &other)
			requires(!std::is_const_v<value_type>)
			: num_items(other.size()), m_capacity(other.size()), a() {
			this->items = a.allocate(m_capacity);
			std::uninitialized_copy(begin(other), end(other), items);
		}

		constexpr ~KVector()
			requires(std::is_const_v<value_type>)
		{}
		constexpr ~KVector()
			requires(!std::is_const_v<value_type>)
		{
			std::destroy_n(items, num_items);
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
		constexpr const_reference &operator[](size_t index) const {
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
			std::uninitialized_move(begin(*this), end(*this), new_items);
			std::destroy_n(items, num_items);
			a.deallocate(items, m_capacity);
			m_capacity = num;
			items = new_items;
		}

		void push_back(value_type item) {
			reserve(num_items+1);
			new(&items[num_items]) value_type(std::move(item));
			num_items += 1;
		}

		void append(value_type item, size_t count = 1) {
			reserve(num_items + count);
			for (size_t i = 0; i < count; ++i) {
				new (&items[num_items + i]) value_type(std::move(item));
			}
			num_items += count;
		}
		template <size_t N> void append(value_type (&other)[N]) {
			return append(KVector(other, N));
		}
		void append(const_pointer other, size_t len) {
			append(KVector<value_type const, Allocator>(other, len));
		}
		void append(KVector<value_type, Allocator> other)
			requires(!std::is_const_v<value_type>)
		{
			if (m_capacity < (num_items + other.size()) || !items) {
				reserve(num_items + other.size());
			}
			std::uninitialized_copy(begin(other), end(other),
			                        &items[num_items]);
			num_items += other.size();
		}
		void append(KVector<value_type const, Allocator> other)
			requires(std::is_same_v<std::remove_const_t<value_type>, value_type>)
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

		template <size_t N> void operator+=(value_type (&other)[N]) {
			return append(other);
		}
		void operator+=(KVector<value_type, Allocator> other)
			requires(!std::is_const_v<value_type>)
		{ return append(other); }
		void operator+=(KVector<T const, Allocator> other)
			requires(std::is_same_v<std::remove_const_t<value_type>, value_type>)
		{
			return append(other);
		}

		constexpr iterator erase(iterator pos) {
			if (pos == end(*this)) {
				return pos;
			}
			std::destroy_at(pos);
			std::move(pos + 1, end(*this), pos);
			--num_items;
			return pos;
		}
		constexpr iterator erase(iterator first, iterator last) {
			if (first == last) {
				return last;
			}
			std::destroy(first, last);
			std::move(last, end(*this), first);
			num_items -= std::distance(first, last);
			return last;
		}

		constexpr void clear() {
			std::destroy_n(items, num_items);
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

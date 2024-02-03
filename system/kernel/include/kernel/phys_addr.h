/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2023 James McNaughton Felder */
#ifndef _KERNEL_PHYS_ADDR_H
#define _KERNEL_PHYS_ADDR_H

#include <compare>
#include <cstddef>
#include <cstdint>
#include <feline/logger.h>
#include <type_traits>

/* A wrapper for a physical pointer */
template <class T> class PhysAddr {
	public:
		PhysAddr(std::nullptr_t) : ptr(nullptr){};

		/* Create a PhysPtr<T> from a uintptr_t */
		explicit PhysAddr(uintptr_t int_ptr)
			: ptr(reinterpret_cast<T *>(int_ptr)){};

		/* Assign from a uintptr_t */
		PhysAddr &operator=(uintptr_t other) {
			ptr = reinterpret_cast<T *>(other);
			return *this;
		};

		/* Create PhysPtr<T> from PhysPtr<void>.
		 * It's safe because we don't dereference the value,
		 * and immediately put it behind another UserPtr */
		PhysAddr(PhysAddr<void> &other)
			: ptr(static_cast<T *>(other.unsafe_raw_get())){};

		/* Create PhysPtr<void> from PhysPtr<U>.
		 * It's safe because we don't dereference the value,
		 * and immediately put it behind another UserPtr */
		template <typename U>
			requires(std::is_same_v<T, void>)
		PhysAddr(PhysAddr<U> const &other)
			: ptr(static_cast<void *>(other.unsafe_raw_get())){};

		/* Copy constructor and operator */
		PhysAddr(PhysAddr const &other) : ptr(other.unsafe_raw_get()){};
		PhysAddr &operator=(PhysAddr const &other) {
			ptr = other.unsafe_raw_get();
			return *this;
		};

		/* Only use this for printing warnings or when mapping it.
		 * Do not read or write from it! */
		T *unsafe_raw_get() const { return ptr; }

		/* Only use this for printing warnings or when mapping it.
		 * Do not read or write from it! */
		uintptr_t as_int() const { return reinterpret_cast<uintptr_t>(ptr); }

		/* Allow adding and subtracting */
		PhysAddr operator+(uintmax_t num) const {
			/* NOTE: these branches are different, despite looking the same:
			 * if it is void, convert ptr to a uintptr _before_ adding with num,
			 * and then convert to PhysAddr otherwise, add them (get implicit
			 * *sizeof(T)), converting to uintptr_t _afterwards_ This also
			 * applies for operator- */
			if constexpr (std::is_same_v<T, void>) {
				return PhysAddr(reinterpret_cast<uintptr_t>(ptr) + num);
			} else {
				return PhysAddr(reinterpret_cast<uintptr_t>(ptr + num));
			}
		}
		PhysAddr operator-(uintmax_t num) const {
			if constexpr (std::is_same_v<T, void>) {
				return PhysAddr(reinterpret_cast<uintptr_t>(ptr) - num);
			} else {
				return PhysAddr(reinterpret_cast<uintptr_t>(ptr - num));
			}
		}

		ptrdiff_t operator-(PhysAddr<T> const &other) const {
			if constexpr (std::is_same_v<T, void>) {
				return reinterpret_cast<ptrdiff_t>(ptr) - other.as_int();
			} else {
				return reinterpret_cast<ptrdiff_t>(ptr - other.as_int());
			}
		}

	private:
		T *ptr;
};

/* Allow comparisons */
template <typename T, typename U>
auto operator<=>(PhysAddr<T> const &first, PhysAddr<U> const &second) {
	return first.unsafe_raw_get() <=> second.unsafe_raw_get();
}

/* Allow comparisons */
template <typename T, typename U>
bool operator==(PhysAddr<T> const &first, PhysAddr<U> const &second) {
	return first.unsafe_raw_get() == second.unsafe_raw_get();
}

template <typename T>
bool operator==(PhysAddr<T> const &addr, std::nullptr_t ptr) {
	return addr.unsafe_raw_get() == ptr;
}

/* Allow outputting */
template <typename T> inline kout &operator<<(kout &out, PhysAddr<T> ptr) {
	return out << hex(ptr.as_int());
}

#endif // _KERNEL_PHYS_ADDR_H

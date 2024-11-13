/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2023 James McNaughton Felder */
#ifndef _KERNEL_PHYS_ADDR_H
#define _KERNEL_PHYS_ADDR_H

#include <cstddef>
#include <cstdint>
#include <feline/logger.h>
#include <type_traits>

template <typename To, typename From>
concept ConstSafePointerConversion = requires(To to, From from) {
	std::is_pointer_v<To>;
	std::is_pointer_v<From>;
	std::is_const_v<std::remove_pointer_t<To>> ||
		!std::is_const_v<std::remove_pointer_t<From>>;
};

static_assert(ConstSafePointerConversion<const void *, const int *>,
              "Casting from const to const is safe.");
static_assert(ConstSafePointerConversion<void *, int *>,
              "Casting from non-const to non-const is safe.");
static_assert(ConstSafePointerConversion<const void *, int *>,
              "Casting from non-const to const is safe.");
static_assert(ConstSafePointerConversion<void *, const int *>,
              "Casting from const to non-const is NOT safe.");

/* A wrapper for a physical pointer */
template <class T> class PhysAddr {
	public:
		constexpr PhysAddr() : ptr(nullptr) {};
		constexpr PhysAddr(std::nullptr_t) : ptr(nullptr) {};

		/* Create a PhysPtr<T> from a pointer
		 * Try to avoid using, because this is meant to reduce the number of
		 * un-typed physical pointers.
		 */
		explicit constexpr PhysAddr(T *ptr) : ptr(ptr) {};

		/* Create a PhysPtr<T> from a uintptr_t */
		explicit PhysAddr(uintptr_t int_ptr)
			: ptr(reinterpret_cast<T *>(int_ptr)) {};

		/* Assign from a uintptr_t */
		PhysAddr &operator=(uintptr_t other) {
			ptr = reinterpret_cast<T *>(other);
			return *this;
		};

		/* Create PhysPtr<T> from PhysPtr<void>.
		 * It's safe because we don't dereference the value,
		 * and immediately put it behind another UserPtr */
		template <typename U>
			requires(std::is_same_v<std::remove_const_t<U>, void> &&
		             !std::is_same_v<std::remove_const_t<T>, void>)
		constexpr PhysAddr(PhysAddr<U> other)
			: ptr(static_cast<T *>(other.unsafe_raw_get())){};

		/* Create PhysPtr<void> from PhysPtr<U>.
		 * It's safe because we don't dereference the value,
		 * and immediately put it behind another UserPtr */
		template <typename U>
			requires(std::is_same_v<std::remove_const_t<T>, void> &&
		             !std::is_same_v<std::remove_const_t<U>, void> &&
		             ConstSafePointerConversion<T *, U *>)
		constexpr PhysAddr(PhysAddr<U> const other)
			: ptr(static_cast<T *>(other.unsafe_raw_get())){};

		/* Create a PhysPtr<T const> from PhysPtr<T>
		 * It should be safe because we are only adding a const, not removing
		 * any. */
		template <typename U>
			requires ConstSafePointerConversion<T *, U *> &&
		             std::is_same_v<std::remove_const_t<T>,
		                            std::remove_const_t<U>>
		constexpr PhysAddr(PhysAddr<U> const other)
			: ptr(const_cast<T *>(other.unsafe_raw_get())){};

		/* Copy constructor and operator */
		constexpr PhysAddr(PhysAddr const &other)
			: ptr(other.unsafe_raw_get()) {};
		constexpr PhysAddr &operator=(PhysAddr const &other) {
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
			if constexpr (std::is_same_v<std::remove_const_t<T>, void>) {
				return PhysAddr(reinterpret_cast<uintptr_t>(ptr) + num);
			} else {
				return PhysAddr(reinterpret_cast<uintptr_t>(ptr + num));
			}
		}
		PhysAddr operator-(uintmax_t num) const {
			if constexpr (std::is_same_v<std::remove_const_t<T>, void>) {
				return PhysAddr(reinterpret_cast<uintptr_t>(ptr) - num);
			} else {
				return PhysAddr(reinterpret_cast<uintptr_t>(ptr - num));
			}
		}

		ptrdiff_t operator-(PhysAddr<T> const &other) const {
			if constexpr (std::is_same_v<std::remove_const_t<T>, void>) {
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
constexpr auto operator<=>(PhysAddr<T> const &first,
                           PhysAddr<U> const &second) {
	return first.unsafe_raw_get() <=> second.unsafe_raw_get();
}

/* Allow comparisons */
template <typename T, typename U>
constexpr bool operator==(PhysAddr<T> const &first, PhysAddr<U> const &second) {
	return first.unsafe_raw_get() == second.unsafe_raw_get();
}

template <typename T>
constexpr bool operator==(PhysAddr<T> const &addr, std::nullptr_t ptr) {
	return addr.unsafe_raw_get() == ptr;
}

/* Allow outputting */
template <typename T> inline kout &operator<<(kout &out, PhysAddr<T> ptr) {
	return out << hex(ptr.as_int());
}

#endif // _KERNEL_PHYS_ADDR_H

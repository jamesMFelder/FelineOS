/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2023 James McNaughton Felder */
#ifndef _KERNEL_USER_PTR_H
#define _KERNEL_USER_PTR_H

#include <cstddef>
#include <cstdlib>
#include <type_traits>

//FIXME: actually validate instead of just a nullptr check
inline bool validate_user_ptr(void const *ptr, size_t len[[maybe_unused]]) {
	return ptr;
}

/* A wrapper for an untrusted pointer */
template <class T>
class UserPtr {
	public:
		bool is_safe() const { return validate_user_ptr(ptr, sizeof(T)); }
		operator bool() const { return is_safe(); }

		/* Crashes if unsafe, so use .is_safe() or convert to bool */
		T* get() {
			if (!is_safe()) {
				std::abort();
			}
			else {
				return ptr;
			}
		}
		/* Crashes if unsafe, so use .is_safe() or convert to bool */
		operator T() { return get(); }

		/* Only use this for printing warnings!
		 * Do not read or write from it! */
		T * unsafe_raw_get() {
			return ptr;
		}

		/* Create UserPtr<T> from UserPtr<void>.
		 * It's safe because we don't dereference the value,
		 * and immediately put it behind another UserPtr */
		UserPtr(UserPtr<void> &other)
			: ptr(static_cast<T*>(other.unsafe_raw_get())) {};

		/* Create UserPtr<void> from UserPtr<U>.
		 * It's safe because we don't dereference the value,
		 * and immediately put it behind another UserPtr */
		template <typename U>
			requires (std::is_same_v<T, void>)
		UserPtr(UserPtr<U> &other)
		: ptr(static_cast<void*>(other.unsafe_raw_get())) {};

	private:
		T *ptr;
};

#endif // _KERNEL_USER_PTR_H

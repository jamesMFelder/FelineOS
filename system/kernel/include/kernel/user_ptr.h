/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2023 James McNaughton Felder */
#ifndef _KERNEL_USER_PTR_H
#define _KERNEL_USER_PTR_H

#include <cstddef>
#include <cstdlib>

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

	private:
		T *ptr;
};

#endif // _KERNEL_USER_PTR_H

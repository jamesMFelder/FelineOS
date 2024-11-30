/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2023 James McNaughton Felder */
#ifndef _HEADER_H
#define _HEADER_H 1

#include <atomic>
#include <feline/cpp_only.h>

/* A basic spinlock implementation */
class Spinlock {
	public:
		/* Wait to acquire the lock */
		void acquire_lock();
		/* Try to acquire it, but just return if it is already in use */
		[[nodiscard("Did you mean acquire_lock?")]] bool try_acquire_lock();
		/* Release the lock */
		void release_lock();

	private:
		/* The actual lock */
		std::atomic<bool> lock{false};
};

#endif /* _HEADER_H */

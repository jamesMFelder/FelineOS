/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2023 James McNaughton Felder */
#ifndef _HEADER_H
#define _HEADER_H 1

#include <feline/cpp_only.h>

#include <atomic>

/* A basic spinlock implementation */
class Spinlock {
	public:
		/* Wait to acquire the lock */
		void acquire_lock();
		/* Release the lock */
		void release_lock();

	private:
		/* The actual lock */
		std::atomic<bool> lock{false};
};

#endif /* _HEADER_H */

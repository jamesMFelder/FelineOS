// SPDX-License-Identifier: MIT
// Copyright (c) 2021 James McNaughton Felder
#ifndef _HEADER_H
#define _HEADER_H 1

#include <feline/cpp_only.h>

#include <atomic>

//A basic spinlock implimentation
class Spinlock{
	public:
		//Initialize the lock to unlocked
		Spinlock();
		//Wait to aquire the lock
		void aquire_lock();
		//Release the lock
		void release_lock();
	private:
		//The actual lock
		std::atomic<bool> lock;
};

#endif // _HEADER_H

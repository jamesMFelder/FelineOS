// SPDX-License-Identifier: MIT
// Copyright (c) 2021 James McNaughton Felder
#ifndef _HEADER_H
#define _HEADER_H 1

#include <stdatomic.h>

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
		//This is c-style atomics, but I don't have all the headers I need (type_traits) for c++-style
		atomic_flag lock=ATOMIC_FLAG_INIT;
};

#endif // _HEADER_H


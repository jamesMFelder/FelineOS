// SPDX-License-Identifier: MIT
// Copyright (c) 2021 James McNaughton Felder
#ifndef _KERN_MEM_H
#define _KERN_MEM_H 1

#include <cstdint>
#include <feline/fixed_width.h>

//Size of 1 page=granularity of chunks we're handing out
#define PHYS_MEM_CHUNK_SIZE 4_KiB

#ifdef __cplusplus

//Use for manipulating pages instead of void*/uintptr_t
//TODO: support 2MiB pages
class page{
	public:
		//Construct a page (with an optional address)
		//Not supporting a uintptr_t version because this is meant to stop you from using that!
		page(void const * const virt_addr=nullptr);
		page(uintptr_t const virt_addr);

		//This is what we really want: easy iterating through page tables
		page& operator++();
		page operator++(int);
		page& operator--();
		page operator--(int);

		//Quickly get and set
		void *get() const;
		uintptr_t getInt() const;
		void set(const void *newAddr);

		//Implicit get
		operator void * () const;

		//Check if it is null
		//Returns true if null, false if not null
		bool isNull() const;
	private:
		uintptr_t addr=0;
};

enum pmm_results{
	pmm_success,
	pmm_already_used,
	pmm_notused,
	pmm_nomem,
	pmm_null
};

//Results that any of the following functions could return
enum map_results{
	map_success, //Generic success
	map_already_mapped, //virt_addr has already has a mapping
	map_notmapped, //virt_addr has already been unmapped or was never mapped in the first place
	map_no_virtmem,//The virtual address space is all used up (only likely for 32-bit systems)
	map_invalid_option,
	map_no_physmem, //Only used when we can't swap out something
	map_no_perm, //General permission denied
	map_err_kernel_space //You tried to map or unmap a page from kernel space as a user program
};


//Aquire len unused bytes
pmm_results get_mem_area(void **addr, uintptr_t len, unsigned int opts);
//Reserve len unused bytes from addr (if available)
pmm_results get_mem_area(void const * const addr, uintptr_t len, unsigned int opts);

//Return len bytes starting at addr(TODO: keep track of who can free what)
pmm_results free_mem_area(void const * const addr, uintptr_t len, unsigned int opts);

//Utility function to turn a number of bytes into a number of pages
//Just a division rounding up
inline uintptr_t constexpr bytes_to_pages(uintptr_t const bytes){
	uintptr_t pages=bytes;
	pages+=PHYS_MEM_CHUNK_SIZE-1;
	pages/=PHYS_MEM_CHUNK_SIZE;
	return pages;
}
inline uintptr_t bytes_to_pages(page const bytes){
	return bytes_to_pages(bytes.getInt());
}

#endif

#endif //_KERN_MEM_H

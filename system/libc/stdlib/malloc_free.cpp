/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2023 James McNaughton Felder */
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <feline/logger.h>
#include <feline/rounding.h>

#if defined(__is_libk)
#include <feline/spinlock.h>
#include <kernel/mem.h>
#endif /* __is_libk */

Spinlock allocation_lock;

static const size_t DEFAULT_MEMRESERVE_SIZE = 4096;
struct Header {
		Header *next;
		Header *prev;
		size_t len = DEFAULT_MEMRESERVE_SIZE;
		bool in_use = false;
		bool start_of_allocation;
		uint8_t canary[2];
};

static Header *first_header;

// Split a chunk of memory into two pieces, the first split bytes long
// and the other containing all the remaining memory
static void split_header(Header *&current_header, size_t split) {
	if (current_header->in_use) {
		kCriticalNoAlloc() << "Header in use! Should not be split!";
		std::abort();
	}
	// FIXME: this makes the Header aligned, but just hopes that the allocation
	// itself doesn't need to be more aligned than the Header
	split = round_to_multiple_of(split, sizeof(Header));
	// This allows a 1-byte other allocation which feels wrong but keeps this
	// generic and symmetric (split can be 1)
	if (current_header->len < split + sizeof(Header)) {
		kCriticalNoAlloc() << "Not enough space after the split for the "
							  "header, let alone any data!";
		std::abort();
	}
	auto *new_header = reinterpret_cast<Header *>(
		reinterpret_cast<uintptr_t>(current_header) + split + sizeof(Header));
	new_header->next = current_header->next;
	new_header->prev = current_header;
	new_header->len = current_header->len - split - sizeof(Header);
	new_header->in_use = false;
	new_header->start_of_allocation = false;
	if (current_header->next) {
		current_header->next->prev = new_header;
	}
	current_header->next = new_header;
	current_header->len = split;
}

// Allocate a new header pointing to needed bytes of memory.
// May add another one after to reduce the number of syscalls.
[[nodiscard]] static Header *allocate_more_mem(size_t needed) {
	Header *hdr;
#ifdef __is_libk
	auto result = get_mem(
		reinterpret_cast<void **>(&hdr),
		round_to_multiple_of(needed + sizeof(Header), DEFAULT_MEMRESERVE_SIZE));
	switch (result) {
	case mem_success:
		break;
	case mem_perm_denied:
		kCriticalNoAlloc() << "No permission to allocate memory?";
		break;
	case mem_invalid:
		kCriticalNoAlloc() << "Invalid argument to allocate memory when we "
							  "didn't specify anything! BUG?";
		break;
	case mem_no_virtmem:
		kCriticalNoAlloc() << "Out of virtual memory!";
		break;
	case mem_no_physmem:
		kCriticalNoAlloc() << "Out of physical memory!";
		break;
	}
	if (result != mem_success) {
		std::abort();
	}
#else  // __is_libk
	static_assert(false, "Memory-allocation syscall not setup yet!");
#endif // __is_libk (else)
	/* If we get here, hdr was initialized in the call to get_mem */
	/* cppcheck-suppress uninitvar */
	hdr->next = nullptr;
	hdr->prev = nullptr;
	hdr->len = DEFAULT_MEMRESERVE_SIZE - sizeof(Header);
	hdr->in_use = false;
	hdr->start_of_allocation = true;
	if (needed < hdr->len - sizeof(Header)) {
		split_header(hdr, needed);
	}
	return hdr;
}

static void return_mem(Header *hdr) {
#ifdef __is_libk
	auto result = free_mem(hdr, hdr->len + sizeof(Header));
	if (result != mem_success) {
		std::abort();
	}
#else  // __is_libk
	static_assert(false, "Memory-deallocation syscall not setup yet!");
#endif // __is_libk (else)
}

void *malloc(size_t size) {
	allocation_lock.aquire_lock();
	if (first_header == nullptr) {
		first_header = allocate_more_mem(size);
	}
	Header *hdr = first_header;
	while (hdr->in_use || hdr->len < size) {
		if (!hdr->next) {
			hdr->next = allocate_more_mem(size);
			hdr->next->prev = hdr;
		}
		hdr = hdr->next;
	}
	if (hdr->len > size + sizeof(Header)) {
		split_header(hdr, size);
	}
	hdr->in_use = true;
	assert(hdr->in_use);
	allocation_lock.release_lock();
	return hdr + 1;
}

void free(void *ptr) {
	if (ptr == nullptr) {
		return;
	}
	allocation_lock.aquire_lock();
	// Nothing has been allocated or everything has been freed!
	if (first_header == nullptr) {
		std::abort();
	}
	Header *hdr = first_header;
	while (hdr->next && hdr + 1 != ptr) {
		hdr = hdr->next;
	}
	// Couldn't find the pointer to free
	if (hdr + 1 != ptr) {
		kCriticalNoAlloc() << "Pointer was not malloc()ed!";
		std::abort();
	}
	// It's already been freed!
	if (!hdr->in_use) {
		kCriticalNoAlloc() << "Pointer has already been free()d!";
		std::abort();
	}
	hdr->in_use = false;
	memset(hdr + 1, 0xDD, hdr->len);
	if (hdr->next && !hdr->next->in_use && !hdr->next->start_of_allocation &&
	    hdr->next ==
	        reinterpret_cast<Header *>(reinterpret_cast<uintptr_t>(hdr) +
	                                   hdr->len + sizeof(Header))) {
		if (hdr->next->next) {
			hdr->next->next->prev = hdr;
		}
		hdr->len += hdr->next->len + sizeof(Header);
		hdr->next = hdr->next->next;
	}
	if (!hdr->start_of_allocation && hdr->prev && hdr->prev->in_use == false &&
	    hdr ==
	        reinterpret_cast<Header *>(reinterpret_cast<uintptr_t>(hdr->prev) +
	                                   hdr->prev->len + sizeof(Header))) {
		hdr->prev->next = hdr->next;
		hdr->prev->len += hdr->len + sizeof(Header);
		if (hdr->next) {
			hdr->next->prev = hdr->prev;
		}
		hdr = hdr->prev;
	}
	while (hdr->start_of_allocation &&
	       hdr->len >= DEFAULT_MEMRESERVE_SIZE - sizeof(Header)) {
		if (hdr->len == DEFAULT_MEMRESERVE_SIZE - sizeof(Header)) {
			if (hdr->prev) {
				hdr->prev->next = hdr->next;
			}
			if (hdr->next) {
				hdr->next->prev = hdr->prev;
			}
			if (hdr == first_header) {
				first_header = hdr->next;
			}
			return_mem(hdr);
			break;
		} else {
			Header *to_del = hdr;
			hdr += DEFAULT_MEMRESERVE_SIZE / sizeof(Header);
			hdr->next = to_del->next;
			hdr->prev = to_del->prev;
			hdr->len = to_del->len - DEFAULT_MEMRESERVE_SIZE;
			hdr->in_use = false;
			hdr->start_of_allocation = true;
			if (to_del->prev) {
				to_del->prev->next = hdr;
			}
			if (to_del->next) {
				to_del->next->prev = hdr;
			}
			if (to_del == first_header) {
				first_header = hdr;
			}
			return_mem(to_del);
			break;
		}
	}
	allocation_lock.release_lock();
}

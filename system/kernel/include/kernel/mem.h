/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2023 James McNaughton Felder */
#ifndef _KERN_MEM_H
#define _KERN_MEM_H 1

#include <cstdint>
#include <feline/fixed_width.h>
#include <kernel/phys_addr.h>

/* Size of 1 page=granularity of chunks we're handing out */
constexpr unsigned long long PHYS_MEM_CHUNK_SIZE = 4_KiB;
constexpr unsigned long long LARGE_CHUNK_SIZE = 1_MiB;

/* Use for manipulating pages instead of void* / uintptr_t */
/* TODO: support 2MiB pages */
class page {
	public:
		/* Construct a page (with an optional address) */
		/* Not supporting a uintptr_t version because this is meant to stop you
		 * from using that! */
		page(void const *const virt_addr = nullptr);
		page(uintptr_t const virt_addr);

		/* This is what we really want: easy iterating through page tables */
		page &operator++();
		page operator++(int);
		page &operator--();
		page operator--(int);

		/* And sometimes we need to do arithmetic */
		page &operator+=(const page &rhs);
		friend page operator+(page lhs, const page &rhs);
		page &operator-=(const page &rhs);
		friend page operator-(page lhs, const page &rhs);

		/* Quickly get and set */
		void *get() const;
		uintptr_t getInt() const;
		void set(const void *newAddr);

		/* Arithmetic */
		page operator+(uintptr_t);
		page operator-(uintptr_t);

		/* Implicit get */
		operator void *() const;

		/* Check if it is null */
		/* Returns true if null, false if not null */
		bool isNull() const;

	protected:
		inline virtual unsigned long long get_chunk_size() const {
			return PHYS_MEM_CHUNK_SIZE;
		};

	private:
		uintptr_t addr = 0;
};

/* Use for manipulating pages instead of void* / uintptr_t */
/* 1MiB version */
class largePage : public page {
	public:
		largePage(void const *const virt_addr = nullptr);
		largePage(uintptr_t const virt_addr);

	protected:
		inline unsigned long long get_chunk_size() const override {
			return LARGE_CHUNK_SIZE;
		};
};

enum pmm_results { pmm_success, pmm_invalid, pmm_nomem, pmm_null };

/* Results that any of the following functions could return */
enum map_results {
	map_success,        /* Generic success */
	map_already_mapped, /* virt_addr has already has a mapping */
	map_notmapped, /* virt_addr has already been unmapped or was never mapped in
	                  the first place */
	map_no_virtmem, /* The virtual address space is all used up (only likely for
	                   32-bit systems) */
	map_invalid_option,
	map_no_physmem,       /* Only used when we can't swap out something */
	map_no_perm,          /* General permission denied */
	map_err_kernel_space, /* You tried to map or unmap a page from kernel space
	                         as a user program */
	map_invalid_align,    /* phys_addr and virt_addr are at different offsets in
	                         their respective pages */
};

enum mem_results {
	mem_success,     /* generic success */
	mem_perm_denied, /* you don't have permission */
	mem_invalid, /* physical or virtual memory is in use already (if allocating)
	                or not (if freeing) */
	mem_no_virtmem, /* you ran out of space in virtual memory */
	mem_no_physmem, /* there is no physical memory left (TODO: impliment swap)
	                 */
};

mem_results get_mem_from(PhysAddr<void> phys_addr, void **new_virt_addr,
                         uintptr_t len);
mem_results get_mem(void **new_virt_addr, uintptr_t len);
mem_results free_mem(void *addr, uintptr_t len);

/* Aquire len unused bytes */
pmm_results get_mem_area(PhysAddr<void const> *addr, uintptr_t len);
/* Reserve len unused bytes from addr (if available) */
pmm_results get_mem_area(PhysAddr<void const> const addr, uintptr_t len);

/* Return len bytes starting at addr(TODO: keep track of who can free what) */
pmm_results free_mem_area(PhysAddr<void const> const addr, uintptr_t len);

/* Utility function to turn a number of bytes into a number of pages */
/* Just a division rounding up with overflow checking. */
inline uintptr_t constexpr bytes_to_pages(uintptr_t const bytes) {
	uintptr_t pages = bytes;
	if (UINTPTR_MAX - pages < PHYS_MEM_CHUNK_SIZE - 1) {
		return UINTPTR_MAX / PHYS_MEM_CHUNK_SIZE;
	}
	pages += PHYS_MEM_CHUNK_SIZE - 1;
	pages /= PHYS_MEM_CHUNK_SIZE;
	return pages;
}
inline uintptr_t bytes_to_pages(page const bytes) {
	return bytes_to_pages(bytes.getInt());
}
/* And largePage versions */
inline uintptr_t constexpr bytes_to_large_pages(uintptr_t const bytes) {
	uintptr_t pages = bytes;
	if (UINTPTR_MAX - pages < LARGE_CHUNK_SIZE - 1) {
		return UINTPTR_MAX / LARGE_CHUNK_SIZE;
	}
	pages += LARGE_CHUNK_SIZE - 1;
	pages /= LARGE_CHUNK_SIZE;
	return pages;
}
inline uintptr_t bytes_to_large_pages(largePage const bytes) {
	return bytes_to_pages(bytes.getInt());
}

#endif /* _KERN_MEM_H */

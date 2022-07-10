/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2022 James McNaughton Felder */

#ifndef _KERN_VTOPMEM_H
#define _KERN_VTOPMEM_H 1


/* How far is the kernel in virtual memory from where it is in physical memory */
#define VA_OFFSET 0xf0000000

/* Only define templates for C++, but don't error if another language includes this file */
#if defined(__cplusplus) && !defined(__ASSEMBLER__)

#include <cstdlib>
#include <kernel/paging.h>
#include <kernel/log.h>

/* Return the value at addr in physical memory, aborting if an error occurs */
template <class T>
T read_pmem(T const * const addr) {
	T *tmp_ptr;
	T value;
	map_results type_mapping=map_range(addr, sizeof(T), reinterpret_cast<void**>(&tmp_ptr), 0);
	if (type_mapping != map_success) {
		kerrorf ("Cannot map physical memory %p. Error %d.", static_cast<void const * const>(addr), type_mapping);
		abort();
	}
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wuninitialized" /* Actually, map_range makes tmp_ptr valid */
	value = *tmp_ptr;
#pragma GCC diagnostic pop
	unmap_range(tmp_ptr, sizeof(T), 0);
	return value;
}

/* Copy num elements from addr(physical memory) to result(virtual memory), aborting if an error occurs */
template <class T>
void read_pmem(T const * const addr, size_t num, T * result) {
	T *tmp_ptr;
	map_results type_mapping=map_range(addr, sizeof(T) * num, reinterpret_cast<void**>(&tmp_ptr), 0);
	if (type_mapping != map_success) {
		kerrorf ("Cannot map physical memory %p. Error %d.", static_cast<void const * const>(addr), type_mapping);
		abort();
	}
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wuninitialized" /* Actually, map_range makes tmp_ptr valid */
	memmove(result, tmp_ptr, sizeof(T) * num);
#pragma GCC diagnostic pop
	unmap_range(tmp_ptr, sizeof(T) * num, 0);
}

/* Write value(virtual memory) to addr(physical memory), aborting if an error occurs */
template <class T>
void write_pmem(T * const addr, T value) {
	T *tmp_ptr;
	map_results type_mapping=map_range(addr, sizeof(T), &tmp_ptr, 0);
	if (type_mapping != map_success) {
		kerrorf ("Cannot map physical memory %p. Error %d.", static_cast<void const * const>(addr), type_mapping);
		abort();
	}
	*tmp_ptr=value;
	unmap_range(tmp_ptr, sizeof(T), 0);
}

/* Copy num elements from value(virtual memory) to addr(physical memory), aborting if an error occurs */
template <class T>
void write_pmem(T * addr, size_t num, T * value) {
	T *tmp_ptr;
	map_results type_mapping=map_range(addr, sizeof(T) * num, reinterpret_cast<void**>(&tmp_ptr), 0);
	if (type_mapping != map_success) {
		kerrorf ("Cannot map physical memory %p. Error %d.", static_cast<void const * const>(addr), type_mapping);
		abort();
	}
	memmove(tmp_ptr, value, sizeof(T) * num);
	unmap_range(tmp_ptr, sizeof(T) * num, 0);
}

#endif /* __cplusplus && !__ASSEMBLER__*/

#endif /* _KERN_VTOPMEM_H */

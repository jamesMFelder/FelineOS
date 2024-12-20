/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2023 James McNaughton Felder */

#ifndef _KERN_VTOPMEM_H
#define _KERN_VTOPMEM_H 1

/* How far is the kernel in virtual memory from where it is in physical memory
 */
#if defined(__i386__)
#define VA_OFFSET 0xf0000000
#elif defined(__arm__)
#define VA_OFFSET 0xf0000000
#else
#error "Unknown architecture! Can't figure out where to put the kernel!"
#endif

/* Only define templates for C++, but don't error if another language includes
 * this file */
#if defined(__cplusplus) && !defined(__ASSEMBLER__)

#include <cstddef>
#include <cstdlib>
#include <kernel/log.h>
#include <kernel/paging.h>
#include <kernel/phys_addr.h>

/* Return the value at addr in physical memory, aborting if an error occurs */
template <class T>
	requires(std::is_const_v<T>)
T read_pmem(PhysAddr<T> const addr) {
	T *tmp_ptr;
	// TODO: should volatile T imply MAP_DEVICE
	map_results type_mapping = map_range(
		addr, sizeof(T), reinterpret_cast<void const **>(&tmp_ptr), 0);
	if (type_mapping != map_success) {
		kerrorf("Cannot map physical memory %p. Error %d.",
		        addr.unsafe_raw_get(), type_mapping);
		std::abort();
	}
	T value = *tmp_ptr;
	unmap_range(tmp_ptr, sizeof(T), 0);
	return value;
}
template <class T> T read_pmem(PhysAddr<T> const addr) {
	return read_pmem<T const>(addr);
}

/* Copy num elements from addr(physical memory) to result(virtual memory),
 * aborting if an error occurs */
template <class T>
void read_pmem(PhysAddr<T> const addr, size_t num, T *result) {
	T *tmp_ptr;
	map_results type_mapping = map_range(
		addr, sizeof(T) * num, reinterpret_cast<void **>(&tmp_ptr), 0);
	if (type_mapping != map_success) {
		kerrorf("Cannot map physical memory %p. Error %d.",
		        addr.unsafe_raw_get(), type_mapping);
		std::abort();
	}
	memmove(result, tmp_ptr, sizeof(T) * num);
	unmap_range(tmp_ptr, sizeof(T) * num, 0);
}

/* Write value(virtual memory) to addr(physical memory), aborting if an error
 * occurs */
template <class T> void write_pmem(PhysAddr<T> const addr, T value) {
	T *tmp_ptr;
	map_results type_mapping =
		map_range(addr, sizeof(T), reinterpret_cast<void **>(&tmp_ptr), 0);
	if (type_mapping != map_success) {
		kerrorf("Cannot map physical memory %p. Error %d.",
		        addr.unsafe_raw_get(), type_mapping);
		std::abort();
	}
	*tmp_ptr = value;
	unmap_range(tmp_ptr, sizeof(T), 0);
}

/* Copy num elements from value(virtual memory) to addr(physical memory),
 * aborting if an error occurs */
template <class T> void write_pmem(PhysAddr<T> addr, size_t num, T *value) {
	T *tmp_ptr;
	map_results type_mapping = map_range(
		addr, sizeof(T) * num, reinterpret_cast<void **>(&tmp_ptr), 0);
	if (type_mapping != map_success) {
		kerrorf("Cannot map physical memory %p. Error %d.",
		        addr.unsafe_raw_get(), type_mapping);
		std::abort();
	}
	memmove(tmp_ptr, value, sizeof(T) * num);
	unmap_range(tmp_ptr, sizeof(T) * num, 0);
}

/* Prints the page tables */
void dump_pagetables();

/* Only take the address of these! */
extern char const kernel_start;
extern char const kernel_end;

extern std::byte _phys_kernel_start;
inline PhysAddr<void> phys_kernel_start(&_phys_kernel_start);

extern std::byte _phys_kernel_end;
inline PhysAddr<void> phys_kernel_end(&_phys_kernel_end);

#endif /* __cplusplus && !__ASSEMBLER__*/

#endif /* _KERN_VTOPMEM_H */

/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2023 James McNaughton Felder */
#ifndef KERNEL_PHYS_MEM_H
#define KERNEL_PHYS_MEM_H

#include <cstddef>

/* Used for a stack of pages */
typedef struct phys_mem_area{
	/* Is any part of this in use (including through a child) */
	bool in_use;
} phys_mem_area_t;

/* Used for passing in information from the bootloader */
struct bootloader_mem_region {
	void *addr;
	size_t len;
};

int start_phys_mem_manager(
		struct bootloader_mem_region *unavailable_memory_regions,
		size_t num_unavailable_memory_regions,
		struct bootloader_mem_region *available_memory_regions,
		size_t num_available_memory_regions
		);

#endif // KERNEL_PHYS_MEM_H

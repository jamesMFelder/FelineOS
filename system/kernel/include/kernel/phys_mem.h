/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2023 James McNaughton Felder */
#ifndef KERNEL_PHYS_MEM_H
#define KERNEL_PHYS_MEM_H

#include <cstddef>
#include <kernel/phys_addr.h>

/* Used for passing in information from the bootloader */
struct bootloader_mem_region {
		PhysAddr<void const> addr;
		size_t len;
};

int start_phys_mem_manager(
	struct bootloader_mem_region *unavailable_memory_regions,
	size_t num_unavailable_memory_regions,
	struct bootloader_mem_region *available_memory_regions,
	size_t num_available_memory_regions);

void ensure_not_allocatable(PhysAddr<void> addr, size_t len);

#endif // KERNEL_PHYS_MEM_H

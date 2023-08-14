/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2023 James McNaughton Felder */
#include "mem.h"
#include <cassert>
#include <kernel/log.h>
#include <kernel/paging.h>
#include <kernel/vtopmem.h>
#include <kernel/mem.h>
#include <kernel/phys_mem.h>
#include <kernel/devicetree.h>
#include <cstring>
#include <cstdlib>
#include <cinttypes>
#include <feline/align.h>
#include <feline/fixed_width.h>
#include <feline/bool_int.h>
#include <feline/minmax.h>

/* Setup by the linker to be at the start and end of the kernel. */
extern const char phys_kernel_start;
extern const char phys_kernel_end;
extern const char kernel_start;
extern const char kernel_end;
const uintptr_t uint_kernel_start = reinterpret_cast<uintptr_t>(&kernel_start);
const uintptr_t uint_kernel_end = reinterpret_cast<uintptr_t>(&kernel_end);
// const uintptr_t phys_uint_kernel_start = reinterpret_cast<uintptr_t>(&phys_kernel_start);
// const uintptr_t phys_uint_kernel_end = reinterpret_cast<uintptr_t>(&phys_kernel_end);
fdt_header *devicetree_header;

/* TODO: make a generic system for registering reserved memory */
struct reserved_mem {
	void const *start;
	void const *end;
};

enum find_actions {
	fail, /* There is no way to make it work */
	success, /* It works perfectley */
	after, /* Try at the end of the current reserved area */
};

static void* find_space_in_area (void const *location, void const *end_of_available, size_t size_needed, size_t alignment_needed) {
	auto is_overlap = [location, size_needed, end_of_available](void const *start_reserved, void const *end_reserved) -> find_actions {
		/* If there is any overlap */
		if (
				(location >= start_reserved && location <= end_reserved)
				||
				(
				 reinterpret_cast<unsigned char const*>(location)+size_needed >= start_reserved
				 &&
				 reinterpret_cast<unsigned char const*>(location)+size_needed < end_reserved
				)
		   ) {
			/* And we do not fit after */
			if (reinterpret_cast<unsigned char*>(const_cast<void*>(end_reserved))+size_needed > end_of_available) {
				/* Say we cannot use this area */
				return fail;
			}
			/* Try to fit after */
			/* And make sure that nothing else conflicts */
			return after;
		}
		/* For this obstacle, there is no overlap.
		 * However, something else might be in the way, so keep checking */
		return success;
	};
	/* Don't use unaligned memory (unpredictable results) */
	location=round_up_to_alignment(location, alignment_needed);
	/* Check device-tree reserved area */
	fdt_reserve_entry *reserved_mem=reinterpret_cast<fdt_reserve_entry*>(devicetree_header+devicetree_header->off_mem_rsvmap/sizeof(*devicetree_header));
	while (reserved_mem->address!=0 || reserved_mem->size!=0) {
		switch (is_overlap(
					reinterpret_cast<void*>(static_cast<uintptr_t>(reserved_mem->address)),
					reinterpret_cast<void*>(static_cast<uintptr_t>(reserved_mem->address+reserved_mem->size))
					)) {
			case fail:
				return nullptr;
			case after:
				return find_space_in_area(
						reinterpret_cast<void*>(static_cast<uintptr_t>(reserved_mem->address+reserved_mem->size)+1),
						end_of_available, size_needed, alignment_needed
						);
			case success:
				break;
		}
		++reserved_mem;
	}
	/* Nothing conflicted with the address, so we're good to go*/
	return const_cast<void*>(location);
};

/* Start the physical memory manager */
/* len=number of memory areas */
/* Create a stack of pages for use */
/* Create+fill in the bitmap */
/* Call after paging is active */
int bootstrap_phys_mem_manager(fdt_header *devicetree){

	klog("Starting bootstrap_phys_mem_manager.");
	devicetree_header = devicetree;

	/* Print debugging information */
	klogf("Kernel starts at %p", static_cast<void const *>(&kernel_start));
	klogf("Kernel ends at %p", static_cast<void const *>(&kernel_end));
	klogf("It is %#" PRIxPTR " bytes long.", uint_kernel_end-uint_kernel_start);

	/* Find the number of entries */
	fdt_reserve_entry *reserved_mem=reinterpret_cast<fdt_reserve_entry*>(devicetree+devicetree->off_mem_rsvmap/sizeof(*devicetree));
	size_t num_reserved_entries=0;
	while (reserved_mem->address!=0 || reserved_mem->size!=0) {
		klogf("Reserved memory at %#" PRIx64 " for %#" PRIx64 " bytes.", static_cast<uint64_t>(reserved_mem->address), static_cast<uint64_t>(reserved_mem->size));
		++num_reserved_entries;
		++reserved_mem;
	}
	klogf("%zu entries found.", num_reserved_entries);
	size_t num_available_entries=0;
	auto print_available_memory = [](fdt_struct_entry *entry, devicetree_cell_size sizes[[maybe_unused]], char *strings, void *state){
		if (strcmp("reg", strings+entry->prop.nameoff)==0) {
			klogf("Available memory at %#" PRIx32 " for %#" PRIx32 " bytes.", static_cast<uint32_t>(entry->prop.value[0]), static_cast<uint32_t>(entry->prop.value[1]));
			(*reinterpret_cast<size_t*>(state))+=1;
		}
		// klogf("Sizes: addresses=%#" PRIx32 ", sizes=%#" PRIx32, sizes.address_cells, sizes.size_cells);
	};
	for_each_prop_in_node("memory", print_available_memory, reinterpret_cast<void*>(&num_available_entries));
	size_t sorted_length=(num_reserved_entries*2+num_available_entries)*sizeof(bootloader_mem_region);
	klogf("There are %zu available memory ranges and %zu reserved memory ranges", num_available_entries, num_reserved_entries);
	klogf("so we will need a maximum of %zu bytes of memory to sort them.", sorted_length);

	struct placement_details {
		size_t len_needed; //Input
		void *where; //Output
	};
	auto can_contain_memory_map = [](fdt_struct_entry *entry, devicetree_cell_size sizes[[maybe_unused]], char *strings, void *state) {
		placement_details *memory_map_location=reinterpret_cast<placement_details*>(state);
		void *addr=reinterpret_cast<void*>(static_cast<uintptr_t>(entry->prop.value[0]));
		size_t len=entry->prop.value[1];
		if (strcmp("reg", strings+entry->prop.nameoff)==0) {
			klogf("Memory at %p for %#zx bytes.", addr, len);
			if (!memory_map_location->where) {
				/* Check if we overlap any data structure that we need to preserve */
				memory_map_location->where=find_space_in_area(addr, reinterpret_cast<char const*>(addr)+len, memory_map_location->len_needed, alignof(bootloader_mem_region));
			}
		}
		// klogf("Sizes: addresses=%#" PRIx32 ", sizes=%#" PRIx32, sizes.address_cells, sizes.size_cells);
	};
	placement_details memory_map_location = {.len_needed=sorted_length, .where=nullptr};
	for_each_prop_in_node("memory", can_contain_memory_map, &memory_map_location);
	if (memory_map_location.where) {
		klogf("Found %#zx bytes at %p.", memory_map_location.len_needed, memory_map_location.where);
	}
	else {
		kcritical("Unable to find a location for bootstrapping the PMM! Aborting.");
		std::abort();
	}

	size_t num_unavailable_regions=0;
	bootloader_mem_region *unavailable_regions=reinterpret_cast<bootloader_mem_region*>(memory_map_location.where);

	/* Reusing the same variable for the same loop */
	reserved_mem=reinterpret_cast<fdt_reserve_entry*>(devicetree_header+devicetree_header->off_mem_rsvmap/sizeof(*devicetree_header));
	while (reserved_mem->address!=0 || reserved_mem->size!=0) {
		unavailable_regions[num_unavailable_regions].addr=reinterpret_cast<void*>(static_cast<uintptr_t>(reserved_mem->address));
		if (reserved_mem->size > SIZE_MAX) {
			std::abort();
		}
		unavailable_regions[num_unavailable_regions].len=static_cast<size_t>(reserved_mem->size);
		++num_unavailable_regions;
		++reserved_mem;
	}
	unavailable_regions[num_unavailable_regions].addr=const_cast<char*>(&phys_kernel_start);
	/* Since phys_kernel_start and phys_kernel_end are setup in the linker file,
	 * parsing the c++ code makes them look unrelated */
	/* cppcheck-suppress comparePointers */
	unavailable_regions[num_unavailable_regions].len=static_cast<size_t>(&phys_kernel_end-&phys_kernel_start);
	++num_unavailable_regions;
	unavailable_regions[num_unavailable_regions].addr=(&devicetree);
	unavailable_regions[num_unavailable_regions].len=devicetree->totalsize;
	++num_unavailable_regions;

	struct available_memory_status {
		bootloader_mem_region *start_of_available;
		size_t num_regions;
	};
	auto add_available_memory_region = [](fdt_struct_entry *entry, devicetree_cell_size sizes[[maybe_unused]], char *strings, void *state) {
		available_memory_status *memory_map_state=reinterpret_cast<available_memory_status*>(state);
		void *addr=reinterpret_cast<void*>(static_cast<uintptr_t>(entry->prop.value[0]));
		size_t len=entry->prop.value[1];
		if (strcmp("reg", strings+entry->prop.nameoff)==0) {
			klogf("Memory at %p for %#zx bytes.", addr, len);
			memory_map_state->start_of_available[memory_map_state->num_regions].addr=addr;
			memory_map_state->start_of_available[memory_map_state->num_regions].len=len;
			++memory_map_state->num_regions;
		}
		// klogf("Sizes: addresses=%#" PRIx32 ", sizes=%#" PRIx32, sizes.address_cells, sizes.size_cells);
	};
	available_memory_status available_memory_state = {.start_of_available=&unavailable_regions[num_unavailable_regions], .num_regions=0};
	for_each_prop_in_node("memory", add_available_memory_region, &available_memory_state);
	klog("Starting physical memory manager!");
	/* Setup the actual physical memory manager */
	return start_phys_mem_manager(unavailable_regions, num_unavailable_regions, available_memory_state.start_of_available, available_memory_state.num_regions);
}

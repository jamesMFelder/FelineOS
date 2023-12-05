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
PhysAddr<void> phys_ptr_kernel_start = nullptr;
PhysAddr<void> phys_ptr_kernel_end = nullptr;
extern const char kernel_start;
extern const char kernel_end;
PhysAddr<fdt_header> phys_devicetree_header = nullptr;
fdt_header *devicetree_header;

/* TODO: make a generic system for registering reserved memory */
enum find_actions {
	fail, /* There is no way to make it work */
	success, /* It works perfectley */
	after, /* Try at the end of the current reserved area */
};

static PhysAddr<void> find_space_in_area (PhysAddr<void> location, PhysAddr<void> end_of_available, size_t size_needed, size_t alignment_needed) {
	auto is_overlap = [location, size_needed, end_of_available](PhysAddr<void> start_reserved, PhysAddr<void> end_reserved) -> find_actions {
		/* If there is any overlap */
		if (
				(location >= start_reserved && location <= end_reserved)
				||
				(
				 location+size_needed >= start_reserved
				 &&
				 location+size_needed < end_reserved
				)
		   ) {
			/* And we do not fit after */
			if (end_reserved+size_needed > end_of_available) {
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
	location=round_up_to_alignment(location.as_int(), alignment_needed);
	/* Check device-tree reserved area */
	fdt_reserve_entry *reserved_mem=reinterpret_cast<fdt_reserve_entry*>(devicetree_header+devicetree_header->off_mem_rsvmap/sizeof(*devicetree_header));
	while (reserved_mem->address!=0 || reserved_mem->size!=0) {
		switch (is_overlap(
					PhysAddr<void>(reserved_mem->address),
					PhysAddr<void>(reserved_mem->address+reserved_mem->size)
					)) {
			case fail:
				return nullptr;
			case after:
				return find_space_in_area(
						PhysAddr<void>(reserved_mem->address+reserved_mem->size+1),
						end_of_available, size_needed, alignment_needed
						);
			case success:
				break;
		}
		++reserved_mem;
	}
	switch (is_overlap(phys_ptr_kernel_start, phys_ptr_kernel_end)) {
		case fail:
			return nullptr;
		case after:
			return find_space_in_area(phys_ptr_kernel_end+1, end_of_available, size_needed, alignment_needed);
		case success:
			break;
	}
	switch (is_overlap(phys_devicetree_header, phys_devicetree_header+devicetree_header->totalsize/sizeof(fdt_header))) {
		case fail:
			return nullptr;
		case after:
			return find_space_in_area(phys_devicetree_header+devicetree_header->totalsize/sizeof(fdt_header)+1, end_of_available, size_needed, alignment_needed);
		case success:
			break;
	}
	++reserved_mem;
	/* Nothing conflicted with the address, so we're good to go*/
	return location;
};

/* Start the physical memory manager */
/* len=number of memory areas */
/* Create a stack of pages for use */
/* Create+fill in the bitmap */
/* Call after paging is active */
int bootstrap_phys_mem_manager(PhysAddr<fdt_header> devicetree){

	phys_devicetree_header = devicetree;
	devicetree_header = init_devicetree(devicetree);
	phys_ptr_kernel_start = reinterpret_cast<uintptr_t>(&phys_kernel_start);
	phys_ptr_kernel_end = reinterpret_cast<uintptr_t>(&phys_kernel_end);

	/* Find the number of entries */
	fdt_reserve_entry *reserved_mem=reinterpret_cast<fdt_reserve_entry*>(devicetree_header+devicetree_header->off_mem_rsvmap/sizeof(fdt_header));
	size_t num_reserved_entries=0;
	while (reserved_mem->address!=0 || reserved_mem->size!=0) {
		++num_reserved_entries;
		++reserved_mem;
	}
	size_t num_available_entries=0;
	auto print_available_memory = [](fdt_struct_entry *entry, devicetree_cell_size sizes[[maybe_unused]], char *strings, void *state){
		if (strcmp("reg", strings+entry->prop.nameoff)==0) {
			(*reinterpret_cast<size_t*>(state))+=1;
		}
	};
	for_each_prop_in_node("memory", print_available_memory, reinterpret_cast<void*>(&num_available_entries));
	//reserved*2 for worst-case (available entry in the middle of each)
	//+3 for kernel, devicetree and the range we are currently reserving
	size_t sorted_length=(num_reserved_entries*2+num_available_entries+3)*sizeof(bootloader_mem_region);

	/*
	 * MASSIVE HACK ALERT!!!
	 * nullptr is 0x0, and 0x0 is a valid location before we enable paging
	 * we can't set found_space to nullptr as default,
	 * because we check it against its default at the end to see if we
	 * * jumped out of the loop (so we should continue, using 0x0 for storage)
	 * * or reached the end of it (so we should abort)
	 * so we don't want to abort just because there is enough free space at 0x0
	 */
#define IN_USE_LOCATION phys_ptr_kernel_start
	struct placement_details {
		size_t len_needed; //Input
		PhysAddr<void> where; //Output
	};
	auto can_contain_memory_map = [](fdt_struct_entry *entry, devicetree_cell_size sizes[[maybe_unused]], char *strings, void *state) {
		placement_details *memory_map_location=reinterpret_cast<placement_details*>(state);
		PhysAddr<void> addr=PhysAddr<void>(entry->prop.value[0]);
		size_t len=entry->prop.value[1];
		if (strcmp("reg", strings+entry->prop.nameoff)==0) {
			if (memory_map_location->where == IN_USE_LOCATION) {
				/* Check if we overlap any data structure that we need to preserve */
				memory_map_location->where=find_space_in_area(addr, addr+len, memory_map_location->len_needed, alignof(bootloader_mem_region));
			}
		}
		// klogf("Sizes: addresses=%#" PRIx32 ", sizes=%#" PRIx32, sizes.address_cells, sizes.size_cells);
	};
	placement_details memory_map_location = {.len_needed=sorted_length, .where=IN_USE_LOCATION};
	for_each_prop_in_node("memory", can_contain_memory_map, &memory_map_location);
	/* See above hack alert for we we don't use (!found_space) */
	if (!(memory_map_location.where != IN_USE_LOCATION)) {
		kcritical("Unable to find a location for bootstrapping the PMM! Aborting.");
		std::abort();
	}

	size_t num_unavailable_regions=0;
	bootloader_mem_region *unavailable_regions=nullptr;
	map_results mapping = map_range(memory_map_location.where.unsafe_raw_get(), memory_map_location.len_needed, reinterpret_cast<void**>(&unavailable_regions), 0);
	if (mapping != map_success) {
		kcritical("Unable to map memory to start the PMM! Aborting!");
		std::abort();
	}

	/* Reusing the same variable for the same loop */
	reserved_mem=reinterpret_cast<fdt_reserve_entry*>(devicetree_header+devicetree_header->off_mem_rsvmap/sizeof(*devicetree_header));
	while (reserved_mem->address!=0 || reserved_mem->size!=0) {
		unavailable_regions[num_unavailable_regions].addr=reserved_mem->address;
		if (reserved_mem->size > SIZE_MAX) {
			std::abort();
		}
		unavailable_regions[num_unavailable_regions].len=static_cast<size_t>(reserved_mem->size);
		++num_unavailable_regions;
		++reserved_mem;
	}
	unavailable_regions[num_unavailable_regions].addr=phys_ptr_kernel_start;
	/* Since phys_kernel_start and phys_kernel_end are setup in the linker file,
	 * parsing the c++ code makes them look unrelated */
	/* cppcheck-suppress comparePointers */
	unavailable_regions[num_unavailable_regions].len=(phys_ptr_kernel_end-phys_ptr_kernel_start.as_int()).as_int();
	++num_unavailable_regions;
	unavailable_regions[num_unavailable_regions].addr=phys_devicetree_header;
	unavailable_regions[num_unavailable_regions].len=devicetree_header->totalsize;
	++num_unavailable_regions;
	//FIXME: putting this region here means it never gets freed.
	// How can we make sure it isn't overwritten until start_phys_mem_manager
	// doesn't need it, but is freed after?
	// Remember to decrease sorted_length afterwards.
	unavailable_regions[num_unavailable_regions].addr=memory_map_location.where;
	unavailable_regions[num_unavailable_regions].len=memory_map_location.len_needed;
	++num_unavailable_regions;

	struct available_memory_status {
		bootloader_mem_region *unavailable_regions;
		size_t num_unavailable_regions;
		bootloader_mem_region *start_of_available;
		size_t num_regions;
	};
	auto add_available_memory_region = [](fdt_struct_entry *entry, devicetree_cell_size sizes[[maybe_unused]], char *strings, void *state) {
		available_memory_status *memory_map_state=reinterpret_cast<available_memory_status*>(state);
		PhysAddr<void> addr=PhysAddr<void>(entry->prop.value[0]);
		size_t len=entry->prop.value[1];
		if (strcmp("reg", strings+entry->prop.nameoff)==0) {
			//assuming the unavailable regions don't overlap, otherwise we will ignore the second
			for (size_t i = 0; i < memory_map_state->num_unavailable_regions; ++i) {
				auto &curr_unavail_region = memory_map_state->unavailable_regions[i];
				auto unavailable_end=curr_unavail_region.addr+curr_unavail_region.len;
				bool found_overlapping_region=false;
				// if the available region is completely covered by the unavailable one, skip adding it
				if (curr_unavail_region.addr < addr && unavailable_end > addr+len) {
					found_overlapping_region = true;
				}
				// if the unavailable region starts in the available region, add the part before it
				if (curr_unavail_region.addr > addr && curr_unavail_region.addr < addr+len) {
					memory_map_state->start_of_available[memory_map_state->num_regions].addr = addr;
					memory_map_state->start_of_available[memory_map_state->num_regions].len = curr_unavail_region.addr-addr;
					++memory_map_state->num_regions;
					found_overlapping_region = true;
				}
				// if the unavailable region ends in the available region, add the part after it
				if (unavailable_end > addr && unavailable_end < addr+len) {
					memory_map_state->start_of_available[memory_map_state->num_regions].addr = unavailable_end;
					memory_map_state->start_of_available[memory_map_state->num_regions].len = addr+len-unavailable_end;
					++memory_map_state->num_regions;
					found_overlapping_region = true;
				}
				// the above two cases combine to correctly add two pieces if the unavailable region is a subset of the available one
				if (found_overlapping_region) {
					return;
				}
			}
			// no overlaps were found, add the full region
			memory_map_state->start_of_available[memory_map_state->num_regions].addr=addr;
			memory_map_state->start_of_available[memory_map_state->num_regions].len=len;
			++memory_map_state->num_regions;
		}
		// klogf("Sizes: addresses=%#" PRIx32 ", sizes=%#" PRIx32, sizes.address_cells, sizes.size_cells);
	};
	available_memory_status available_memory_state = {
		.unavailable_regions=unavailable_regions, .num_unavailable_regions=num_unavailable_regions,
		.start_of_available=&unavailable_regions[num_unavailable_regions], .num_regions=0
	};
	for_each_prop_in_node("memory", add_available_memory_region, &available_memory_state);
	/* Setup the actual physical memory manager */
	return start_phys_mem_manager(unavailable_regions, num_unavailable_regions, available_memory_state.start_of_available, available_memory_state.num_regions);
}

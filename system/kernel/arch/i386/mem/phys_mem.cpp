/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2023 James McNaughton Felder */
#include "mem.h"
#include <cassert>
#include <kernel/log.h>
#include <kernel/paging.h>
#include <kernel/vtopmem.h>
#include <kernel/mem.h>
#include <kernel/phys_mem.h>
#include <cstring>
#include <cstdlib>
#include <cinttypes>
#include <feline/fixed_width.h>
#include <feline/bool_int.h>
#include <feline/minmax.h>

/* Advance to the next multiboot memory map entry */
inline multiboot_memory_map_t *next_mmap_entry(multiboot_memory_map_t *mmap_entry){
	return reinterpret_cast<typeof(mmap_entry)>(reinterpret_cast<uintptr_t>(mmap_entry) + mmap_entry->size + sizeof(mmap_entry->size));
}

/* Setup by the linker to be at the start and end of the kernel. */
extern const char phys_kernel_start;
extern const char phys_kernel_end;
extern const char kernel_start;
extern const char kernel_end;
const uintptr_t uint_kernel_start = reinterpret_cast<uintptr_t>(&kernel_start);
const uintptr_t uint_kernel_end = reinterpret_cast<uintptr_t>(&kernel_end);
const uintptr_t phys_uint_kernel_start = reinterpret_cast<uintptr_t>(&phys_kernel_start);
const uintptr_t phys_uint_kernel_end = reinterpret_cast<uintptr_t>(&phys_kernel_end);

/* Start the physical memory manager */
/* mbp=MultiBoot Pointer (everything grub gives us) */
/* mbmp=MultiBoot Memory Pointer (grub lsmmap command output) */
/* len=number of memory areas */
/* Create a stack of pages for use */
/* Create+fill in the bitmap */
/* Call after paging is active */
int bootstrap_phys_mem_manager(multiboot_info_t *phys_mbp){
	multiboot_info_t mbp=read_pmem<multiboot_info_t>(phys_mbp);
	assert(mbp.flags & MULTIBOOT_INFO_MEM_MAP);

	size_t mbmp_len=mbp.mmap_length;
	multiboot_memory_map_t *mbmp;
	map_results mbmp_mapping=map_range(reinterpret_cast<multiboot_memory_map_t*>(mbp.mmap_addr), mbmp_len, reinterpret_cast<void**>(&mbmp), 0);
	assert(mbmp_mapping==map_success);

	klog("Starting bootstrap_phys_mem_manager.");

	/* Print debugging information */
	klogf("Kernel starts at %p", static_cast<void const *>(&kernel_start));
	klogf("Kernel ends at %p", static_cast<void const *>(&kernel_end));
	klogf("It is %#" PRIxPTR " bytes long.", uint_kernel_end-uint_kernel_start);

	/* Find the number of entries */
	size_t num_entries=mbmp_len/sizeof(multiboot_memory_map_t);
	size_t sorted_length=num_entries*sizeof(bootloader_mem_region);
	klogf("There are %zu memory ranges, so we will need a maximum of %zu bytes of memory to sort them.", num_entries, sorted_length);

	/*
	 * MASSIVE HACK ALERT!!!
	 * nullptr is 0x0, and 0x0 is a valid location before we enable paging
	 * we can't set found_space to nullptr as default,
	 * because we check it against its default at the end to see if we
	 * * jumped out of the loop (so we should continue, using 0x0 for storage)
	 * * or reached the end of it (so we should abort)
	 * so we don't want to abort just because there is enough free space at 0x0
	 */
#define IN_USE_LOCATION reinterpret_cast<bootloader_mem_region*>(const_cast<char*>(&phys_kernel_start))
	bootloader_mem_region *found_space=IN_USE_LOCATION;
	/* Search for a space large enough to sort them that isn't being used already */
	multiboot_memory_map_t *current_multiboot_memory=mbmp;
	while (current_multiboot_memory < mbmp+(mbmp_len/sizeof(multiboot_memory_map_t))){
		/* Check if it's available from the hardware/firmware (as reported by GRUB) */
		bool hardware_available = current_multiboot_memory->type==MULTIBOOT_MEMORY_AVAILABLE;
		/* Check if it's large enough */
		bool large_enough = current_multiboot_memory->len>=sorted_length;
		/* Because x86_64 computers can have more than 4GiB of memory installed,
		 * while running 32-bit software (that can only access the first 4GiB normally)
		 * check that we won't overflow a pointer by incrementing it */
		bool no_pointer_wrapping = current_multiboot_memory->addr+sorted_length < (4_GiB-1);
		/* Check if we overlap any data structure that we need to preserve
		 * the commandline is already saved so we don't need to check for it
		 * TODO: what else should we be avoiding */
		/* TODO: can we fit before or after in the same memory region */
		bool avoiding_kernel=current_multiboot_memory->addr>phys_uint_kernel_end || current_multiboot_memory->addr+current_multiboot_memory->len<phys_uint_kernel_end;
		bool avoiding_grub_memmap=reinterpret_cast<multiboot_memory_map_t*>(current_multiboot_memory->addr)>mbmp+(mbmp_len/sizeof(multiboot_memory_map_t)) || reinterpret_cast<multiboot_memory_map_t*>(current_multiboot_memory->addr+current_multiboot_memory->len)<mbmp;
		bool avoiding_module=true; //TODO: check when we support modules
		bool overlapping_data=avoiding_kernel && avoiding_grub_memmap && avoiding_module;
		/* Can we use the space */
		if (hardware_available && large_enough && no_pointer_wrapping && !overlapping_data){
			found_space=reinterpret_cast<bootloader_mem_region*>(current_multiboot_memory->addr);
			break;
		}
		current_multiboot_memory=next_mmap_entry(current_multiboot_memory);
	}
	/* See above hack alert for we we don't use (!found_space) */
	if (found_space==IN_USE_LOCATION) {
		kcriticalf("Cannot find enough memory.");
		std::abort();
	}

	/* Actually map the found space. */
	struct bootloader_mem_region *unavailable_memory;
	enum map_results mapping=map_range(found_space, sorted_length, reinterpret_cast<void**>(&unavailable_memory), 0);
	if (mapping!=map_success) {
		kcriticalf("Unable to map the needed memory for the PMM boostrapping!");
		std::abort();
	}

	/* Copy the multiboot information into our cross-platform setup
	 * For keeping track of where various arrays begin/end
	 * Don't forget to add new variables here when adding a new memory type */
	unavailable_memory[0].addr=found_space;
	unavailable_memory[0].len=sorted_length;
	klogf("Unavailable memory region: at %p\t| length: %#zx", unavailable_memory[0].addr, unavailable_memory[0].len);
	unavailable_memory[1].addr=const_cast<char*>(&phys_kernel_start);
	unavailable_memory[1].len=phys_uint_kernel_end-phys_uint_kernel_start;
	klogf("Unavailable memory region: at %p\t| length: %#zx", unavailable_memory[1].addr, unavailable_memory[1].len);
	size_t num_unavailable_regions=2;

	/* First, copy the number of unavailable entries over */
	current_multiboot_memory=mbmp;
	struct bootloader_mem_region *new_memory=&unavailable_memory[num_unavailable_regions];
	while (current_multiboot_memory<mbmp+(mbmp_len/sizeof(multiboot_memory_map_t))){
		if (current_multiboot_memory->type!=MULTIBOOT_MEMORY_AVAILABLE){
			//If the list is out of order/has overlaps add it in
			if (
							new_memory > unavailable_memory &&
							(current_multiboot_memory->addr)<
							reinterpret_cast<uintptr_t>((new_memory-1)->addr)+
							reinterpret_cast<uintptr_t>((new_memory-1)->len)
			   ) {
				//The list must be sorted up until now, or this breaks
				bool merged=false;
				for (size_t memory_region_index=0; memory_region_index < num_unavailable_regions; ++memory_region_index) {
					/* If the new region starts before the old region ends, and it ends after the old region begins, they can merge. */
					if ((
								(current_multiboot_memory->addr)
								<=
								reinterpret_cast<uintptr_t>(unavailable_memory[memory_region_index].addr)+unavailable_memory[memory_region_index].len
						) && (
								current_multiboot_memory->addr+current_multiboot_memory->len
								>=
								reinterpret_cast<uintptr_t>(unavailable_memory[memory_region_index].addr)
						)) {
						unavailable_memory[memory_region_index].addr = min(reinterpret_cast<void*>(current_multiboot_memory->addr), unavailable_memory[memory_region_index].addr);
						unavailable_memory[memory_region_index].len = static_cast<size_t>(max(
								current_multiboot_memory->addr+current_multiboot_memory->len,
								reinterpret_cast<uintptr_t>(unavailable_memory[memory_region_index].addr)+static_cast<unsigned long long>(unavailable_memory[memory_region_index].len)
								)-reinterpret_cast<uintptr_t>(unavailable_memory[memory_region_index].addr));
						merged=true;
						klogf("merged regions");
						break;
					}
				}
				if (!merged) {
					for (size_t memory_region_index=0; memory_region_index < num_unavailable_regions; ++memory_region_index) {
						/* Move this and everything after it back one spot to put the new memory in here */
						if (reinterpret_cast<uintptr_t>(unavailable_memory[memory_region_index].addr) < current_multiboot_memory->addr) {
							for (size_t moved_index=num_unavailable_regions; moved_index > memory_region_index; --moved_index) {
								unavailable_memory[moved_index]=unavailable_memory[moved_index-1];
							}
							unavailable_memory[memory_region_index].addr=reinterpret_cast<void*>(current_multiboot_memory->addr);
							unavailable_memory[memory_region_index].len=static_cast<size_t>(current_multiboot_memory->len);
							++num_unavailable_regions;
							break;
						}
					}
				}
			}
			//Otherwise, simply append it
			else if (current_multiboot_memory->addr<4_GiB) {
				new_memory->addr=reinterpret_cast<void*>(current_multiboot_memory->addr);
				new_memory->len=static_cast<size_t>(min(current_multiboot_memory->len, 4_GiB-current_multiboot_memory->addr));
				++num_unavailable_regions;
			}
			else {
				kwarnf("Ignoring memory region starting at %#llx.", current_multiboot_memory->addr);
			}
			klogf("Unavailable memory region: at %p\t| length: %#zx", new_memory->addr, new_memory->len);
			++new_memory;
		}

		current_multiboot_memory=next_mmap_entry(current_multiboot_memory);
	}

	/* Finally, copy the number of available entries over
	 * nb. resetting current_multiboot_memory because we skipped available entries
	 *   but not resetting new_memory because we don't want to overwrite the previous lists
	 * TOOD: check consistency with the other list */
	struct bootloader_mem_region *available_memory=new_memory;
	size_t num_available_regions=0;
	current_multiboot_memory=mbmp;
	while (current_multiboot_memory<mbmp+(mbmp_len/sizeof(multiboot_memory_map_t))){
		if (current_multiboot_memory->type==MULTIBOOT_MEMORY_AVAILABLE){
			//If the list is out of order/has overlaps just abort (TODO: sort it)
			if ((current_multiboot_memory->addr)<reinterpret_cast<uintptr_t>((new_memory-1)->addr)+reinterpret_cast<uintptr_t>((new_memory-1)->len)) {
				kcriticalf("Memory map out of order (new start: %#llx < last end: %#" PRIxPTR "). Sorting required but I haven't coded that yet.",
						current_multiboot_memory->addr,
						reinterpret_cast<uintptr_t>((new_memory-1)->addr)
						+reinterpret_cast<uintptr_t>((new_memory-1)->len)
					  );
			}
			//check for overlap with unavailable memory (in which case this region is at least partially invalid)
			//TODO: save the valid areas of the overlapping region
			bool should_drop_region=false; //If a region should be dropped (eg. conflicts with an unavailable region)
			for (size_t i=0; i<num_unavailable_regions; ++i) {
				if (
						current_multiboot_memory->addr > reinterpret_cast<uintptr_t>(unavailable_memory[i].addr) &&
						current_multiboot_memory->addr+current_multiboot_memory->len <
						reinterpret_cast<uintptr_t>(unavailable_memory[i].addr) + unavailable_memory[i].len
				   ){
					kerrorf("Available memory region %#llx-%#llx overlaps with unavailable memory region %p-%#" PRIxPTR ".",
							current_multiboot_memory->addr, current_multiboot_memory->addr+current_multiboot_memory->len,
							unavailable_memory[i].addr, reinterpret_cast<uintptr_t>(unavailable_memory[i].addr)+unavailable_memory[i].len
					       );
					kerror("Dropping available region.");
					should_drop_region=true;
					break;
				}
			}

			if (!should_drop_region){
				new_memory->addr=reinterpret_cast<void*>(current_multiboot_memory->addr);
				new_memory->len=static_cast<size_t>(current_multiboot_memory->len);
				++num_available_regions;
				klogf("Available memory region: at %p\t| length: %#zx", new_memory->addr, new_memory->len);
				new_memory++;
			}
		}

		current_multiboot_memory=next_mmap_entry(current_multiboot_memory);
	}

	unmap_range(mbmp, mbmp_len, 0);

	/* Setup the actual physical memory manager*/
	return start_phys_mem_manager(unavailable_memory, num_unavailable_regions, available_memory, num_available_regions);
}

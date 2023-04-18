/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2023 James McNaughton Felder */
#include <cassert>
#include <cinttypes>
#include <cstdlib>
#include <cstring>
#include <feline/align.h>
#include <feline/bool_int.h>
#include <feline/fixed_width.h>
#include <feline/minmax.h>
#include <feline/spinlock.h>
#include <kernel/log.h>
#include <kernel/mem.h>
#include <kernel/paging.h>
#include <kernel/phys_mem.h>
#include <kernel/vtopmem.h>

/* Setup by the linker to be at the start and end of the kernel. */
extern const char phys_kernel_start;
extern const char phys_kernel_end;
extern const char kernel_start;
extern const char kernel_end;
const uintptr_t uint_kernel_start = reinterpret_cast<uintptr_t>(&kernel_start);
const uintptr_t uint_kernel_end = reinterpret_cast<uintptr_t>(&kernel_end);

Spinlock modifying_pmm;

static phys_mem_area_t *normal_mem_bitmap = nullptr;
size_t mem_bitmap_len = 0;

/* Start the physical memory manager */
/* Create a bitmap of pages for use */
/* Create+fill in the bitmap */
/* Call after paging is active */
int start_phys_mem_manager(
		struct bootloader_mem_region *unavailable_memory_regions,
		size_t num_unavailable_memory_regions,
		struct bootloader_mem_region *available_memory_regions,
		size_t num_available_memory_regions) {

	modifying_pmm.aquire_lock();
	klog("Starting bootstrap_phys_mem_manager.");

	/* Print debugging information */
	klogf("Kernel starts at %p", static_cast<void const *>(&kernel_start));
	klogf("Kernel ends at %p", static_cast<void const *>(&kernel_end));
	klogf("It is %#" PRIxPTR " bytes long.",
			uint_kernel_end - uint_kernel_start);

	/* Find the required size of the bitmap (maximum physical memory we can use) */
	// TODO: how to safely not overflow
	uintmax_t avail_max_mem =
		reinterpret_cast<uintmax_t>(available_memory_regions[num_available_memory_regions - 1].addr) +
		available_memory_regions[num_available_memory_regions - 1].len;
	uintmax_t unavail_max_mem =
		reinterpret_cast<uintmax_t>(unavailable_memory_regions[num_unavailable_memory_regions - 1].addr) +
		unavailable_memory_regions[num_unavailable_memory_regions - 1].len;
	uintmax_t max_mem=max(avail_max_mem, unavail_max_mem);
	max_mem=min(max_mem, 4_GiB-1);

	mem_bitmap_len=bytes_to_pages(max_mem);
	printf("%#zx = min(%#" PRIxPTR ", %#" PRIxPTR ")\n", mem_bitmap_len, bytes_to_pages(4_GiB-1), bytes_to_pages(max_mem));

	klogf("Using %zu elements in array.", mem_bitmap_len);
	size_t needed_size=(mem_bitmap_len+1)*sizeof(phys_mem_area_t);

	/* Is it possible to fit size_needed bytes into location..end_of_available without overlapping something important
	 * If so, where in the area can we go; if not, nullptr */
	auto find_space_in_area = [&unavailable_memory_regions, &num_unavailable_memory_regions, &available_memory_regions, &num_available_memory_regions](void const *location, void const *end_of_available, size_t size_needed, auto& recursive_self) -> phys_mem_area_t* {
		/* Align the area (faster on x86, required on arm) */
		location=round_up_to_alignment(location, alignof(phys_mem_area_t));
		enum find_actions {
			fail, /* There is no way to make it work */
			success, /* It works perfectley */
			after, /* Try at the end of the current reserved area */
		};
		/* Check if a location works */
		auto is_overlap = [location, size_needed, end_of_available](void const *start_reserved, void const *end_reserved) -> find_actions {
			/* See if there is no overlap */
			if (
					location > end_reserved ||
					reinterpret_cast<unsigned char const*>(location)+size_needed < start_reserved
			   ) {
				return success;
			}
			/* Otherwise, see if there is enough room after */
			else if (reinterpret_cast<unsigned char const*>(end_reserved)+size_needed < end_of_available) {
				return after;
			}
			/* If nothing worked, return failure */
			else {
				return fail;
			}
		};
		/* For each possible conflict */
		for (size_t i=0; i<num_unavailable_memory_regions; ++i) {
			/* Check if they overlap */
			switch (is_overlap(
						unavailable_memory_regions[i].addr,
						reinterpret_cast<void*>(
							reinterpret_cast<uintptr_t>(unavailable_memory_regions[i].addr)
							+unavailable_memory_regions[i].len)
						)
				   ) {
				/* If they don't overlap, check the next possible conflict */
				case success:
					continue;
				/* If they do overlap, and there isn't space after, there isn't space at all */
				case fail:
					return nullptr;
				/* If they do overlap, but there is space after, try again starting after the conflict */
				case after:
					return recursive_self(
							reinterpret_cast<void*>(
								reinterpret_cast<uintptr_t>(unavailable_memory_regions[i].addr)
								+unavailable_memory_regions[i].len+1
								),
							end_of_available, size_needed, recursive_self
							);
			}
		}
		/* And the bootloader_mem_regions */
		switch (is_overlap(unavailable_memory_regions, unavailable_memory_regions+num_unavailable_memory_regions)) {
			/* If they don't overlap, check the next possible conflict */
			case success:
				break;
			/* If they do overlap, and there isn't space after, there isn't space at all */
			case fail:
				return nullptr;
			/* If they do overlap, but there is space after, try again starting after the conflict */
			case after:
				return recursive_self(unavailable_memory_regions+num_unavailable_memory_regions+1, end_of_available, size_needed, recursive_self);
		}
		switch (is_overlap(available_memory_regions, available_memory_regions+num_available_memory_regions)) {
			/* If they don't overlap, check the next possible conflict */
			case success:
				break;
			/* If they do overlap, and there isn't space after, there isn't space at all */
			case fail:
				return nullptr;
			/* If they do overlap, but there is space after, try again starting after the conflict */
			case after:
				return recursive_self(available_memory_regions+num_available_memory_regions+1, end_of_available, size_needed, recursive_self);
		}
		/* Because we reached the end of the loop and did the switch for everything else,
		 * nothing conflicted with the address, so it works */
		return reinterpret_cast<phys_mem_area_t*>(const_cast<void*>(location));
	};
	/* For each available memory region, check if we can use it */
	for (size_t i=0; i<num_available_memory_regions; ++i) {
		normal_mem_bitmap=find_space_in_area(
				available_memory_regions[i].addr,
				reinterpret_cast<void*>(
					reinterpret_cast<uintptr_t>(available_memory_regions[i].addr)
					+available_memory_regions[i].len
					),
				needed_size, find_space_in_area);
		/* If it's not nullptr, we don't need to check anywhere else because it points to a good place */
		if (normal_mem_bitmap) {
			break;
		}
	}

	if (normal_mem_bitmap==nullptr) {
		kcritical("Unable to find space for the memory bitmap. Aborting!");
		abort();
	}

	/* Note where we found the memory and stop searching */
	klog("");
	klogf("Bitmap at %p.", static_cast<void*>(normal_mem_bitmap));
	klogf("Ends at %p.", static_cast<void*>(normal_mem_bitmap+mem_bitmap_len));
	klog("");

	/* Map it, saving it's address to mark it reserved later. */
	uintptr_t phys_nmb=reinterpret_cast<uintptr_t>(normal_mem_bitmap);
	printf("Bitmap points to %p in physical memory...",
			reinterpret_cast<void*>(normal_mem_bitmap));
	map_results mapping=map_range(normal_mem_bitmap,
			mem_bitmap_len*sizeof(phys_mem_area_t),
			reinterpret_cast<void**>(&normal_mem_bitmap), 0);
	if(mapping!=map_success){
		puts("");
		kcriticalf("Unable to map the physical memory manager (error code %d).", mapping);
		abort();
	}
	printf("and to %p in virtual memory.\n", reinterpret_cast<void*>(normal_mem_bitmap));
	/* Fill in the bitmap */
	auto set_region_to = [max_mem](void *addr, size_t len, bool in_use) {
		size_t max_trackable_len=max_mem-reinterpret_cast<uintptr_t>(addr);
		len=min(len, max_trackable_len);
		if (bytes_to_pages(len) > mem_bitmap_len) {
			kerrorf("Requesting %#zx pages of memory (max %#zx).", bytes_to_pages(len), mem_bitmap_len);
			abort();
		}

		/* TODO: delete low-level logging */
		printf("Setting %p to %s for %#" PRIxPTR " bytes.\n",
				static_cast<void*>(addr),
				in_use?"unavailable":"available",
				bytes_to_pages(len));
		/* fill in the array */
		std::memset(&normal_mem_bitmap[bytes_to_pages(addr)],
				in_use,
				bytes_to_pages(len)/sizeof(*normal_mem_bitmap));
	};
	/* Quickly mark unknown parts of the bitmap as unuseable */
	set_region_to(0x0, max_mem, true);
	for (size_t mem_index=0; mem_index < num_available_memory_regions; ++mem_index) {
		set_region_to(available_memory_regions[mem_index].addr, available_memory_regions[mem_index].len, false);
	}
	for (size_t mem_index=0; mem_index < num_unavailable_memory_regions; ++mem_index) {
		set_region_to(unavailable_memory_regions[mem_index].addr, unavailable_memory_regions[mem_index].len, true);
	}

	/* Unmap the architecture-specific memory info */
	unmap_range(unavailable_memory_regions,
			(num_available_memory_regions + num_unavailable_memory_regions) *
			sizeof(struct bootloader_mem_region), 0);

	puts("Bitmap filled...Marking kernel as used.");
	/* Now mark the bitmap as used */
	std::memset(&normal_mem_bitmap[phys_nmb/PHYS_MEM_CHUNK_SIZE], INT_TRUE, bytes_to_pages(mem_bitmap_len));
	/* And the first page (so it can be nullptr) */
	normal_mem_bitmap[0].in_use=true;
	modifying_pmm.release_lock();
	klog("Ending bootstrap_phys_mem_manager.");
	return 0;
}

/* Find num unused (contiguous) pages */
/* modifying_pmm must be locked before calling this! */
/* returns nullptr if no RAM is available */
static page find_free_pages(size_t num) {
	/* Abort if we don't have enough total RAM */
	if (num >= mem_bitmap_len) {
		abort();
	}
	/* Loop through the page table stopping when there can't be enough free
	 * space
	 */
	/* Start at 1, because nullptr is 0 and means not enough space was available
	*/
	for (size_t where = 1; where < mem_bitmap_len - num; where++) {
		/* If it is free */
		if (!normal_mem_bitmap[where].in_use) {
			/* Start counting up to the number we need */
			for (size_t count = 0; count < num; count++) {
				/* If one isn't free */
				if (normal_mem_bitmap[where + count].in_use) {
					/* Go back to searching for a free one after this */
					where += count;
					break;
				}
			}
			/* If the free space wasn't long enough */
			if (normal_mem_bitmap[where].in_use) {
				/* continue looking */
				continue;
			}
			/* There is enough free space!! */
			/* Loop through again */
			else {
				return where * PHYS_MEM_CHUNK_SIZE;
			}
		}
	}
	/* There isn't enough memory */
	return nullptr;
}

/* Reserve num pages starting at addr */
/* modifying_pmm must be locked before calling this! */
static pmm_results internal_claim_mem_area(page const addr, size_t num, unsigned int opts [[maybe_unused]]) {
	/* Make sure we are given a valid pointer */
	if (addr.isNull()) {
		return pmm_null;
	}
	/* Get the offset into the bitmap */
	size_t offset = bytes_to_pages(addr);
	/* Check if someone is trying to access beyond the end of RAM */
	if (offset > mem_bitmap_len - num) {
		return pmm_nomem;
	}
	/* Double check that everything is free and claim it */
	for (size_t count = 0; count < num; count++) {
		/* If it is in use */
		if (normal_mem_bitmap[offset + count].in_use) {
			/* Roll back our changes */
			while (count > 0) {
				normal_mem_bitmap[offset + count - 1].in_use = false;
				count--;
			}
			/* And return an error */
			return pmm_invalid;
		}
		/* It is free */
		else {
			/* Claim it */
			normal_mem_bitmap[offset + count].in_use = true;
		}
	}
	return pmm_success;
}

/* Reserve len unused bytes from addr (if available) */
pmm_results get_mem_area(void const *const addr, uintptr_t len, unsigned int opts) {
	modifying_pmm.aquire_lock();
	pmm_results temp = internal_claim_mem_area(addr, bytes_to_pages(len), opts);
	modifying_pmm.release_lock();
	return temp;
}

/* Aquire len unused bytes */
pmm_results get_mem_area(void **addr, uintptr_t len, unsigned int opts) {
	modifying_pmm.aquire_lock();
	*addr = find_free_pages(bytes_to_pages(len));
	if (*addr == nullptr) {
		modifying_pmm.release_lock();
		return pmm_nomem;
	}
	pmm_results result =
		internal_claim_mem_area(*addr, bytes_to_pages(len), opts);
	modifying_pmm.release_lock();
	return result;
}

/* Free len bytes from addr */
pmm_results free_mem_area(void const *const addr, uintptr_t len, unsigned int opts [[maybe_unused]]) {
	modifying_pmm.aquire_lock();
	/* Figure out how many pages to mark free */
	size_t num = bytes_to_pages(len);
	page base_page = addr;
	size_t base_index = base_page.getInt() / PHYS_MEM_CHUNK_SIZE;
	/* Loop through all the pages */
	for (size_t count = 0; count < num; count++) {
		/* And make sure that none of them are already free */
		if (!normal_mem_bitmap[base_index + count].in_use) {
			/* If any are, quit immediatly */
			modifying_pmm.release_lock();
			return pmm_invalid;
		}
	}
	/* Loop through again */
	for (size_t count = 0; count < num; count++) {
		/* And unmap everything */
		/* TODO: zero the pages? */
		normal_mem_bitmap[base_index + count].in_use = false;
	}
	modifying_pmm.release_lock();
	return pmm_success;
}

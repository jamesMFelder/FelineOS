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

Spinlock modifying_pmm;

/* Return the offset into a page */
inline uintptr_t page_offset(uintptr_t const addr){
	return addr&(PHYS_MEM_CHUNK_SIZE-1);
}
inline uintptr_t page_offset(void const * const addr){
	return page_offset(reinterpret_cast<uintptr_t>(addr));
}

page round_up_to_page(uintptr_t len) {
	len += PHYS_MEM_CHUNK_SIZE-1;
	len &= ~(PHYS_MEM_CHUNK_SIZE-1);
	return len;
}
page addr_round_up_to_page(void *len) {
	return round_up_to_page(reinterpret_cast<uintptr_t>(len));
}

// static phys_mem_area_t *normal_mem_bitmap = nullptr;
// size_t mem_bitmap_len = 0;

struct PhysMemHeader {
	PhysMemHeader *next;
	PhysMemHeader *prev;
	page memory;
	page size; //number of PHYS_MEM_CHUNK_SIZE chunks (we will lose the ends of areas that don't nicely fit, which is okay)
	bool in_use;
	uint8_t canary[3];
};

static PhysMemHeader *first_header = nullptr;

int start_phys_mem_manager(
		struct bootloader_mem_region *unavailable_memory_regions,
		size_t num_unavailable_memory_regions,
		struct bootloader_mem_region *available_memory_regions,
		size_t num_available_memory_regions) {
	modifying_pmm.aquire_lock();
	/* +2 if we go in the middle of a memory region, +1 so we can bootstrap adding more */
	size_t num_headers_needed = num_available_memory_regions+3;
	size_t headers_needed_size = round_up_to_page(num_headers_needed*sizeof(PhysMemHeader)).getInt();

	klogf("Using %zu memory regions.", num_headers_needed);

	/* Is it possible to fit size_needed bytes into location..end_of_available without overlapping something important
	 * If so, where in the area can we go; if not, nullptr */
	auto find_space_in_area = [&unavailable_memory_regions, &num_unavailable_memory_regions, &available_memory_regions, &num_available_memory_regions](void const *location, void const *end_of_available, size_t size_needed, auto& recursive_self) -> PhysMemHeader* {
		/* Align the area to the minimum chunk alignment we can reserve */
		location=round_up_to_alignment(location, PHYS_MEM_CHUNK_SIZE);
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
		return reinterpret_cast<PhysMemHeader*>(const_cast<void*>(location));
	};
	/* For each available memory region, check if we can use it */
	for (size_t i=0; i<num_available_memory_regions; ++i) {
		first_header=find_space_in_area(
				available_memory_regions[i].addr,
				reinterpret_cast<void*>(
					reinterpret_cast<uintptr_t>(available_memory_regions[i].addr)
					+available_memory_regions[i].len
					),
				headers_needed_size, find_space_in_area);
		/* If it's not nullptr, we don't need to check anywhere else because it points to a good place */
		if (first_header) {
			break;
		}
	}

	if (first_header==nullptr) {
		kcritical("Unable to find space for the memory headers. Aborting!");
		std::abort();
	}

	/* Note where we found the memory and stop searching */
	klog("");
	klogf("Headers start at %p.", static_cast<void*>(first_header));
	klogf("Headers end at %p.", static_cast<void*>(first_header+num_headers_needed));
	klog("");

	/* Map it, saving it's address to mark it reserved later. */
	unsigned char *phys_hdr_ptr=reinterpret_cast<unsigned char*>(first_header);
	printf("Header chunk is at %p in physical memory...",
			reinterpret_cast<void*>(first_header));
	map_results mapping=map_range(first_header,
			headers_needed_size,
			reinterpret_cast<void**>(&first_header), 0);
	if(mapping!=map_success){
		puts("");
		kcriticalf("Unable to map the physical memory manager (error code %d).", mapping);
		std::abort();
	}
	printf("and %p in virtual memory.\n", reinterpret_cast<void*>(first_header));

	/* Copy the available bootloader_mem_regions to the linked list */
	/* Don't bother keeping the unavailable ones, because we can never free them */
	size_t cur_avail_region_offset=0, cur_header_offset=0;
	/* Loop while we still have a header to copy over */
	while (cur_avail_region_offset < num_available_memory_regions) {
		/* Verify we're not copying to much (most likely from splitting a region to include the headers) */
		if (cur_header_offset >= num_headers_needed) {
			kcritical("BUG: miscalculated the number of headers needed for the PMM!");
			std::abort();
		}
		if (available_memory_regions[cur_avail_region_offset].addr <= phys_hdr_ptr && \
				static_cast<unsigned char*>(available_memory_regions[cur_avail_region_offset].addr)+available_memory_regions[cur_avail_region_offset].len >= \
				(phys_hdr_ptr + headers_needed_size)) {
			if (available_memory_regions[cur_avail_region_offset].addr != first_header) {
				first_header[cur_header_offset].next = nullptr;
				first_header[cur_header_offset].prev = &first_header[cur_header_offset-1];
				first_header[cur_header_offset].memory = addr_round_up_to_page(available_memory_regions[cur_avail_region_offset].addr);
				first_header[cur_header_offset].size = phys_hdr_ptr - addr_round_up_to_page(available_memory_regions[cur_header_offset].addr).getInt();
				first_header[cur_header_offset].in_use = false;
				++cur_header_offset;
			}
			first_header[cur_header_offset].next = nullptr;
			first_header[cur_header_offset].prev = &first_header[cur_header_offset-1];
			first_header[cur_header_offset].memory = first_header;
			first_header[cur_header_offset].size = headers_needed_size;
			first_header[cur_header_offset].in_use = true;
			++cur_header_offset;
			if (static_cast<unsigned char*>(available_memory_regions[cur_avail_region_offset].addr)+available_memory_regions[cur_avail_region_offset].len != \
					first_header[cur_header_offset-1].memory+first_header[cur_header_offset-1].size) {
				first_header[cur_header_offset].next = nullptr;
				first_header[cur_header_offset].prev = &first_header[cur_header_offset-1];
				first_header[cur_header_offset].memory = first_header[cur_header_offset-1].memory+first_header[cur_header_offset-1].size;
				first_header[cur_header_offset].size = static_cast<unsigned char*>(available_memory_regions[cur_avail_region_offset].addr)+available_memory_regions[cur_avail_region_offset].len-\
													   (first_header[cur_header_offset-1].memory+first_header[cur_header_offset-1].size).getInt();
				first_header[cur_header_offset].in_use = false;
				++cur_header_offset;
			}
		}
		else {
			first_header[cur_header_offset].next = nullptr;
			first_header[cur_header_offset].prev = &first_header[cur_header_offset-1];
			first_header[cur_header_offset].memory = addr_round_up_to_page(available_memory_regions[cur_avail_region_offset].addr);
			first_header[cur_header_offset].size = available_memory_regions[cur_avail_region_offset].len-page_offset(available_memory_regions[cur_avail_region_offset].addr);
			first_header[cur_header_offset].in_use = false;
		}
		++cur_avail_region_offset;
		++cur_header_offset;
	}
	/* Avoid walking off the start of the chain */
	first_header[0].prev = nullptr;
	/* And reserve the location the headers are currently at */
	modifying_pmm.release_lock();
	return 0;
}

enum allocation_strategy {
	first_fit,
};

/* Find num unused (contiguous) pages */
/* modifying_pmm must be locked before calling this! */
/* returns nullptr if no RAM is available */
static PhysMemHeader* find_free_pages(size_t len, allocation_strategy strategy[[maybe_unused]]) {
	for (PhysMemHeader *cur_header = first_header; cur_header != nullptr; cur_header = cur_header->next) {
		/* If it is available and there is enough space */
		if (!cur_header->in_use && cur_header->size.getInt() >= len) {
			return cur_header;
		}
	}
	/* There isn't enough memory */
	return nullptr;
}

/* Find the header for addr, making sure it is available and has enough space */
static PhysMemHeader* find_header_for_page(void const * const addr, size_t len) {
	for (PhysMemHeader *cur_header = first_header; cur_header != nullptr; cur_header = cur_header->next) {
		if (cur_header->memory <= addr && cur_header->memory+cur_header->size >= addr) {
			/* We found the header, now we just need to check if we can use it */
			if (cur_header->in_use || cur_header->size.getInt() < len) {
				return nullptr;
			}
			else {
				return cur_header;
			}
		}
	}
	/* We couldn't find the header */
	return nullptr;
}

/* Reserve num pages that addr controls */
/* modifying_pmm must be locked before calling this! */
static pmm_results internal_claim_mem_area(PhysMemHeader &header, size_t len) {
	assert(!header.in_use);
	assert(header.size.getInt() >= len);
	header.in_use = true;
	return pmm_success;
}

/* Reserve len unused bytes from addr (if available) */
pmm_results get_mem_area(void const *const addr, uintptr_t len) {
	modifying_pmm.aquire_lock();
	PhysMemHeader *header = find_header_for_page(addr, len);
	if (!header) {
		return pmm_nomem;
	}
	pmm_results temp = internal_claim_mem_area(*header, len);
	modifying_pmm.release_lock();
	return temp;
}

/* Aquire len unused bytes */
pmm_results get_mem_area(void **addr, uintptr_t len) {
	modifying_pmm.aquire_lock();
	auto *header = find_free_pages(len, first_fit);
	if (header == nullptr) {
		modifying_pmm.release_lock();
		return pmm_nomem;
	}
	pmm_results result =
		internal_claim_mem_area(*header, len);
	if (result==pmm_success) {
		*addr = header->memory;
	}
	modifying_pmm.release_lock();
	return result;
}

/* Free len bytes from addr */
pmm_results free_mem_area(void const *const addr, uintptr_t len) {
	modifying_pmm.aquire_lock();
	for (PhysMemHeader *cur_header = first_header; cur_header != nullptr; cur_header = cur_header->next) {
		if (cur_header->memory <= addr && cur_header->memory+cur_header->size >= addr) {
			auto end = static_cast<unsigned char const *>(addr)+len;
			if (cur_header->memory+cur_header->size < end) {
				/* We only know about part of the area? */
				kwarnf("Attempt to free area %p-%p, but header covers area %p-%p!", addr, end, cur_header->memory.get(), (cur_header->memory+cur_header->size).get());
				/* TODO: should we abort? */
			}
			if (!cur_header->in_use) {
				modifying_pmm.release_lock();
				return pmm_invalid;
			}
			cur_header->in_use = false;
			/* TODO: merge with free regions */
			modifying_pmm.release_lock();
			return pmm_success;
		}
	}
	modifying_pmm.release_lock();
	/* We never found the header, so it was an invalid free */
	return pmm_invalid;
}

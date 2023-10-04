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
#include <kernel/settings.h>

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

struct PhysMemHeader {
	PhysMemHeader *next;
	PhysMemHeader *prev;
	page memory;
	page size; //number of PHYS_MEM_CHUNK_SIZE chunks (we will lose the ends of areas that don't nicely fit, which is okay)
	bool in_use;
	uint8_t canary[2];
	bool header_in_use; //is the header controlling memory? (NOT the same as if the memory it points to is in use)
};

struct PhysMemHeaderList {
	PhysMemHeaderList *next;
	PhysMemHeader headers[(PHYS_MEM_CHUNK_SIZE-sizeof(PhysMemHeaderList*))/sizeof(PhysMemHeader)];
};

static PhysMemHeader *first_header = nullptr;
static PhysMemHeaderList *first_chunk = nullptr;

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
	auto find_space_in_area = [&unavailable_memory_regions, &num_unavailable_memory_regions, &available_memory_regions, &num_available_memory_regions](void const *location, void const *end_of_available, size_t size_needed, auto& recursive_self) -> PhysMemHeaderList* {
		/* Align the area to the minimum chunk alignment we can reserve */
		location=round_up_to_alignment(location, PHYS_MEM_CHUNK_SIZE);
		enum find_actions {
			fail, /* There is no way to make it work */
			success, /* It works perfectly */
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
		return reinterpret_cast<PhysMemHeaderList*>(const_cast<void*>(location));
	};
	/* For each available memory region, check if we can use it */
	for (size_t i=0; i<num_available_memory_regions; ++i) {
		first_chunk=find_space_in_area(
				available_memory_regions[i].addr,
				reinterpret_cast<void*>(
					reinterpret_cast<uintptr_t>(available_memory_regions[i].addr)
					+available_memory_regions[i].len
					),
				headers_needed_size, find_space_in_area);
		/* If it's not nullptr, we don't need to check anywhere else because it points to a good place */
		if (first_chunk) {
			break;
		}
	}

	if (first_chunk==nullptr) {
		kcritical("Unable to find space for the memory headers. Aborting!");
		std::abort();
	}

	/* Note where we found the memory and stop searching */
	klog("");
	klogf("Headers start at %p.", static_cast<void*>(first_chunk));
	klogf("Headers end at %p.", static_cast<void*>(first_chunk+num_headers_needed));
	klog("");

	/* Map it, saving it's address to mark it reserved later. */
	unsigned char *phys_hdr_ptr=reinterpret_cast<unsigned char*>(first_chunk);
	printf("Header chunk is at %p in physical memory...",
			reinterpret_cast<void*>(first_chunk));
	map_results mapping=map_range(first_chunk,
			headers_needed_size,
			reinterpret_cast<void**>(&first_chunk), 0);
	if(mapping!=map_success){
		kcriticalf("\nUnable to map the physical memory manager (error code %d).", mapping);
		std::abort();
	}
	printf("and %p in virtual memory.\n", reinterpret_cast<void*>(first_chunk));
	first_chunk->next = nullptr;
	for (auto &hdr : first_chunk->headers) {
		/* Clear each field (size=0-0 to choose integer overload instead of void* one for page constructor) */
		hdr = {.next=nullptr, .prev=nullptr, .memory=nullptr, .size=0-0, .in_use=false, .canary={0, 0}, .header_in_use=false};
	}
	first_header = &first_chunk->headers[0];

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
				first_header[cur_header_offset].header_in_use = true;
				++cur_header_offset;
			}
			first_header[cur_header_offset].next = nullptr;
			first_header[cur_header_offset].prev = &first_header[cur_header_offset-1];
			first_header[cur_header_offset].memory = first_header;
			first_header[cur_header_offset].size = headers_needed_size;
			first_header[cur_header_offset].in_use = true;
			first_header[cur_header_offset].header_in_use = true;
			++cur_header_offset;
			if (static_cast<unsigned char*>(available_memory_regions[cur_avail_region_offset].addr)+available_memory_regions[cur_avail_region_offset].len != \
					first_header[cur_header_offset-1].memory+first_header[cur_header_offset-1].size) {
				first_header[cur_header_offset].next = nullptr;
				first_header[cur_header_offset].prev = &first_header[cur_header_offset-1];
				first_header[cur_header_offset].memory = first_header[cur_header_offset-1].memory+first_header[cur_header_offset-1].size;
				first_header[cur_header_offset].size = static_cast<unsigned char*>(available_memory_regions[cur_avail_region_offset].addr)+available_memory_regions[cur_avail_region_offset].len-\
													   (first_header[cur_header_offset-1].memory+first_header[cur_header_offset-1].size).getInt();
				first_header[cur_header_offset].in_use = false;
				first_header[cur_header_offset].header_in_use = true;
				++cur_header_offset;
			}
		}
		else {
			first_header[cur_header_offset].next = nullptr;
			first_header[cur_header_offset].prev = &first_header[cur_header_offset-1];
			first_header[cur_header_offset].memory = addr_round_up_to_page(available_memory_regions[cur_avail_region_offset].addr);
			first_header[cur_header_offset].size = available_memory_regions[cur_avail_region_offset].len-page_offset(available_memory_regions[cur_avail_region_offset].addr);
			first_header[cur_header_offset].in_use = false;
			first_header[cur_header_offset].header_in_use = true;
			++cur_header_offset;
		}
		++cur_avail_region_offset;
	}
	/* Avoid walking off the start of the chain */
	first_header[0].prev = nullptr;
	Settings::PMM::totalMem.initialize(static_cast<unsigned long long>(first_header[cur_header_offset-2].memory.getInt())+first_header[cur_header_offset].size.getInt());
	/* And reserve the location the headers are currently at */
	modifying_pmm.release_lock();
	return 0;
}

/* Get the next available header (returns nullptr if none are available) */
static PhysMemHeader* get_new_header() {
	PhysMemHeaderList *chunk = first_chunk;
	/* Go through each chunk of headers */
	while (chunk != nullptr) {
		/* And iterate through each header in that chunk */
		for (auto &header : chunk->headers) {
			/* Returning the header if we can use it */
			if (!header.header_in_use) {
				/* Marking it as used now */
				header.header_in_use = true;
				return &header;
			}
		}
		/* None of the headers were available, go to the next chunk */
		chunk=chunk->next;
	}
	/* None of the chunks had an available header (TODO: allocate a new one) */
	kwarn("Unable to find a new header for the PMM to use!");
	return nullptr;
}

enum allocation_strategy {
	first_fit,
};

/* Find num unused (contiguous) pages */
/* modifying_pmm must be locked before calling this! */
/* returns nullptr if no RAM is available */
static PhysMemHeader* find_free_pages(page len, allocation_strategy strategy[[maybe_unused]]) {
	for (PhysMemHeader *cur_header = first_header; cur_header != nullptr; cur_header = cur_header->next) {
		/* If it is available and there is enough space */
		assert(cur_header->header_in_use);
		if (!cur_header->in_use && cur_header->size >= len) {
			return cur_header;
		}
	}
	/* There isn't enough memory */
	return nullptr;
}

/* Find the header for addr, making sure it is available and has enough space */
static PhysMemHeader* find_header_for_page(void const * const addr, size_t len) {
	for (PhysMemHeader *cur_header = first_header; cur_header != nullptr; cur_header = cur_header->next) {
		assert(cur_header->header_in_use);
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

/* Make the header point to only len amount of memory if possible */
/* No garantees, because allocating a new header might be necessary and fail */
/* However, if it doesn't work, everything is valid, just suboptimal */
static void split_header_at_len(PhysMemHeader &header, page len) {
	assert(header.header_in_use);
	assert(!header.in_use);
	assert(header.size >= len);
	if (header.size == len) {
		return;
	}
	/* Get a new header to hold the split */
	PhysMemHeader *temp = get_new_header();
	/* We didn't get a new header, so don't split, just return extra memory */
	/* This might be important if we're reserving more memory for a new chunk */
	if (temp == nullptr) {
		kwarn("Unable to get a new header for the PMM! Returning extra memory.");
		return;
	}
	/* Intialize the new header */
	*temp = PhysMemHeader{
		.next=header.next,
		.prev=&header,
		.memory=header.memory+len,
		.size=header.size-len,
		.in_use=false,
		.canary={0, 0},
		.header_in_use=true
	};
	/* Add it to the chain */
	if (header.next) {
		header.next->prev = temp;
	}
	header.next = temp;
	/* And change the size on the returned header */
	header.size -= temp->size;
}

/* Reserve the pages that addr controls */
/* modifying_pmm must be locked before calling this! */
static pmm_results internal_claim_mem_area(PhysMemHeader &header) {
	assert(header.header_in_use);
	assert(!header.in_use);
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
	split_header_at_len(*header, round_up_to_page(len));
	pmm_results temp = internal_claim_mem_area(*header);
	modifying_pmm.release_lock();
	return temp;
}

/* Aquire len unused bytes */
pmm_results get_mem_area(void **addr, uintptr_t len) {
	modifying_pmm.aquire_lock();
	auto *header = find_free_pages(round_up_to_page(len), first_fit);
	if (header == nullptr) {
		modifying_pmm.release_lock();
		return pmm_nomem;
	}
	split_header_at_len(*header, round_up_to_page(len));
	pmm_results result =
		internal_claim_mem_area(*header);
	if (result==pmm_success) {
		*addr = header->memory;
	}
	modifying_pmm.release_lock();
	return result;
}

/* Merge the headers next to this one if they are both free */
void merge_adjacent_headers(PhysMemHeader &header) {
	if (header.next && !header.next->in_use && header.memory+header.size == header.next->memory) {
		header.next->header_in_use = false;
		header.size += header.next->size;
		if (header.next->next) {
			header.next->next->prev = &header;
		}
		header.next = header.next->next;
	}
	if (header.prev && !header.prev->in_use && header.prev->memory+header.prev->size == header.memory) {
		header.header_in_use = false;
		header.prev->size += header.size;
		header.prev->next = header.next;
		if (header.next) {
			header.next->prev = header.prev;
		}
	}
}

/* Free len bytes from addr */
pmm_results free_mem_area(void const *const addr, uintptr_t len) {
	modifying_pmm.aquire_lock();
	for (PhysMemHeader *cur_header = first_header; cur_header != nullptr; cur_header = cur_header->next) {
		assert(cur_header->header_in_use);
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
			merge_adjacent_headers(*cur_header);
			/* TODO: merge with free regions */
			modifying_pmm.release_lock();
			return pmm_success;
		}
	}
	modifying_pmm.release_lock();
	/* We never found the header, so it was an invalid free */
	return pmm_invalid;
}

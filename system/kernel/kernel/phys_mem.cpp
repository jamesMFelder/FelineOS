/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2023 James McNaughton Felder */
#include <cassert>
#include <cinttypes>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <feline/align.h>
#include <feline/bool_int.h>
#include <feline/fixed_width.h>
#include <feline/minmax.h>
#include <feline/ranges.h>
#include <feline/settings.h>
#include <feline/spinlock.h>
#include <kernel/log.h>
#include <kernel/mem.h>
#include <kernel/paging.h>
#include <kernel/phys_addr.h>
#include <kernel/phys_mem.h>
#include <kernel/vtopmem.h>

Spinlock modifying_pmm;

/* Return the offset into a page */
inline uintptr_t page_offset(uintptr_t const addr) {
	return addr & (PHYS_MEM_CHUNK_SIZE - 1);
}
inline uintptr_t page_offset(PhysAddr<void const> addr) {
	return page_offset(addr.as_int());
}

inline page round_up_to_page(uintptr_t len) {
	len += PHYS_MEM_CHUNK_SIZE - 1;
	len &= ~(PHYS_MEM_CHUNK_SIZE - 1);
	return len;
}
inline page addr_round_up_to_page(PhysAddr<void const> len) {
	return round_up_to_page(len.as_int());
}

struct PhysMemHeader {
		PhysMemHeader *next;
		PhysMemHeader *prev;
		page memory;
		page size; // number of PHYS_MEM_CHUNK_SIZE chunks (we will lose the
		           // ends of areas that don't nicely fit, which is okay)
		bool in_use;
		uint8_t canary[2];
		bool header_in_use; // is the header controlling memory? (NOT the same
		                    // as if the memory it points to is in use)
};

struct PhysMemHeaderList {
		PhysMemHeaderList *next;
		PhysMemHeader
			headers[(PHYS_MEM_CHUNK_SIZE - sizeof(PhysMemHeaderList *)) /
		            sizeof(PhysMemHeader)];

		/* Allow for loops over the headers */
		typedef PhysMemHeader value_type;
		typedef value_type *iterator;
		iterator data() { return headers; }
		size_t size() { return sizeof(headers) / sizeof(PhysMemHeader); }
		value_type operator[](size_t index) { return headers[index]; }
};
/* Guarantee that this still fits in exactly PHYS_MEM_CHUNK_SIZE bytes */
static_assert(sizeof(PhysMemHeaderList) <= PHYS_MEM_CHUNK_SIZE,
              "PhysMemHeaderList should fit in PHYS_MEM_CHUNK_SIZE!");
static_assert(sizeof(PhysMemHeaderList) + sizeof(PhysMemHeader) >
                  PHYS_MEM_CHUNK_SIZE,
              "PhysMemHeaderList should have as many PhysMemHeader array "
              "elements as possible!");

static PhysMemHeader *first_header = nullptr;
static PhysMemHeaderList *first_chunk = nullptr;

/* For keeping track of if a new PhysMemHeaderList is needed before searching
 * the entire PMM linked list*/
static uintmax_t headers_in_use;
static uintmax_t headers_allocated;

int start_phys_mem_manager(
	struct bootloader_mem_region *unavailable_memory_regions,
	size_t num_unavailable_memory_regions,
	struct bootloader_mem_region *available_memory_regions,
	size_t num_available_memory_regions) {
	modifying_pmm.acquire_lock();
	/* +2 if we go in the middle of a memory region, +1 so we can bootstrap
	 * adding more */
	size_t num_headers_needed = num_available_memory_regions + 3;
	size_t headers_needed_size =
		round_up_to_page(num_headers_needed * sizeof(PhysMemHeader)).getInt();

	/* Is it possible to fit size_needed bytes into location..end_of_available
	 * without overlapping something important If so, where in the area can we
	 * go; if not, nullptr */
	auto find_space_in_area =
		[&unavailable_memory_regions, &num_unavailable_memory_regions](
			PhysAddr<void const> location,
			PhysAddr<void const> end_of_available, size_t size_needed,
			auto &recursive_self) -> PhysAddr<PhysMemHeaderList const> {
		/* Align the area to the minimum chunk alignment we can reserve */
		location =
			round_up_to_alignment(location.as_int(), PHYS_MEM_CHUNK_SIZE);
		enum find_actions {
			fail,    /* There is no way to make it work */
			success, /* It works perfectly */
			after,   /* Try at the end of the current reserved area */
		};
		/* Check if a location works */
		auto is_overlap =
			[location, size_needed, end_of_available](
				PhysAddr<void const> start_reserved,
				PhysAddr<void const> end_reserved) -> find_actions {
			/* See if there is no overlap */
			if (location > end_reserved ||
			    location + size_needed < start_reserved) {
				return success;
			}
			/* Otherwise, see if there is enough room after */
			else if (end_reserved + size_needed < end_of_available) {
				return after;
			}
			/* If nothing worked, return failure */
			else {
				return fail;
			}
		};
		/* For each possible conflict */
		for (size_t i = 0; i < num_unavailable_memory_regions; ++i) {
			/* Check if they overlap */
			switch (is_overlap(unavailable_memory_regions[i].addr,
			                   unavailable_memory_regions[i].addr +
			                       unavailable_memory_regions[i].len)) {
			/* If they don't overlap, check the next possible conflict */
			case success:
				continue;
			/* If they do overlap, and there isn't space after, there isn't
			 * space at all */
			case fail:
				return nullptr;
			/* If they do overlap, but there is space after, try again starting
			 * after the conflict */
			case after:
				return recursive_self(unavailable_memory_regions[i].addr +
				                          unavailable_memory_regions[i].len + 1,
				                      end_of_available, size_needed,
				                      recursive_self);
			}
		}
		/* Because we reached the end of the loop and did the switch for
		 * everything else, nothing conflicted with the address, so it works */
		return location;
	};
	/* For each available memory region, check if we can use it */
	PhysAddr<void const> phys_hdr_ptr = nullptr;
	for (size_t i = 0; i < num_available_memory_regions; ++i) {
		phys_hdr_ptr = find_space_in_area(
			available_memory_regions[i].addr,
			available_memory_regions[i].addr + available_memory_regions[i].len,
			headers_needed_size, find_space_in_area);
		/* If it's not nullptr, we don't need to check anywhere else because it
		 * points to a good place */
		if (phys_hdr_ptr.unsafe_raw_get()) {
			break;
		}
	}

	if (phys_hdr_ptr == nullptr) {
		kcritical("Unable to find space for the memory headers. Aborting!");
		std::abort();
	}

	/* Map it, saving it's address to mark it reserved later. */
	map_results mapping = map_range(phys_hdr_ptr, headers_needed_size,
	                                reinterpret_cast<void **>(&first_chunk), 0);
	if (mapping != map_success) {
		kcriticalf("Unable to map the physical memory manager (error code %d).",
		           mapping);
		std::abort();
	}
	first_chunk->next = nullptr;
	for (auto &hdr : first_chunk->headers) {
		/* Clear each field (size=0-0 to choose integer overload instead of
		 * void* one for page constructor) */
		hdr = {.next = nullptr,
		       .prev = nullptr,
		       .memory = nullptr,
		       .size = 0 - 0,
		       .in_use = false,
		       .canary = {0xCD, 0xEF},
		       .header_in_use = false};
		headers_allocated += 1;
	}
	first_header = &first_chunk->headers[0];

	/* Copy the available bootloader_mem_regions to the linked list */
	/* Don't bother keeping the unavailable ones, because we can never free them
	 */
	size_t cur_header_offset = 0;
	auto try_add_available_memory_region =
		[&unavailable_memory_regions, &num_unavailable_memory_regions,
	     &cur_header_offset](bootloader_mem_region available_region,
	                         auto &recursive_self) -> void {
		for (size_t unavail_offset = 0;
		     unavail_offset < num_unavailable_memory_regions;
		     ++unavail_offset) {
			bootloader_mem_region unavail_region =
				unavailable_memory_regions[unavail_offset];
			auto available_range =
				range{.start = available_region.addr,
			          .end = available_region.addr + available_region.len};
			auto unavailable_range =
				range{.start = unavail_region.addr,
			          .end = unavail_region.addr + unavail_region.len};

			// If the two ranges do not overlap at all, then we're good
			if (!overlap(available_range, unavailable_range)) {
				continue;
			}
			// If the available region is completely covered by an
			// unavailable region, we can't use it
			else if (unavailable_range.start <= available_range.start &&
			         unavailable_range.end >= available_range.end) {
				return;
			}
			// If the unavailable region is a strict subset of the available
			// region, split the available region into two pieces
			else if (unavailable_range.start > available_range.start &&
			         unavailable_range.end < available_range.end) {
				recursive_self(
					bootloader_mem_region{
						.addr = available_region.addr,
						.len = static_cast<size_t>(unavailable_range.start -
				                                   available_range.start -
				                                   PHYS_MEM_CHUNK_SIZE)},
					recursive_self);
				available_region.addr =
					unavailable_range.end + PHYS_MEM_CHUNK_SIZE;
				available_region.len =
					available_range.end - available_region.addr;
			}
			// If it only cuts off the start or end, adjust the range
			else if (unavailable_range.start <= available_range.start &&
			         unavailable_range.end >= available_range.start) {
				available_region.addr =
					unavailable_range.end + PHYS_MEM_CHUNK_SIZE;
				available_region.len = available_range.end -
				                       unavailable_range.end -
				                       PHYS_MEM_CHUNK_SIZE;
			} else if (unavailable_range.start <= available_range.end &&
			           unavailable_range.end >= available_range.end) {
				available_region.len = unavailable_range.end -
				                       available_range.start -
				                       PHYS_MEM_CHUNK_SIZE;
			} else {
				kcritical("What case am I missing in the PMM?");
				std::abort();
			}
		}
		// If it overlaps an existing range, just extend that one
		for (size_t avail_hdr_offset = 0; avail_hdr_offset < cur_header_offset;
		     ++avail_hdr_offset) {
			auto available_range = range{
				.start = available_region.addr.as_int(),
				.end = available_region.addr.as_int() + available_region.len};
			auto prev_found_range =
				range{.start = first_header[avail_hdr_offset].memory.getInt(),
			          .end = first_header[avail_hdr_offset].memory.getInt() +
			                 first_header[avail_hdr_offset].size.getInt()};
			if (overlap(available_range, prev_found_range)) {
				first_header[avail_hdr_offset].memory =
					min(available_range.start, prev_found_range.start);
				first_header[avail_hdr_offset].size =
					max(available_range.end, prev_found_range.end) -
					first_header[avail_hdr_offset].memory;
				return;
				/* TODO: deal with multiple overlaps (a overlaps b, b overlaps
				 * c, but a does not overlap c) If they are checked a->c->b or
				 * c->a->b, then this will lead to double-allocations! */
			}
		}
		if (cur_header_offset != 0) {
			first_header[cur_header_offset].prev =
				&first_header[cur_header_offset - 1];
			first_header[cur_header_offset - 1].next =
				&first_header[cur_header_offset];
		}
		first_header[cur_header_offset].memory = available_region.addr.as_int();
		first_header[cur_header_offset].size = available_region.len;
		first_header[cur_header_offset].in_use = false;
		first_header[cur_header_offset].header_in_use = true;
		++cur_header_offset;
		headers_in_use += 1;
	};
	for (size_t avail_region_offset = 0;
	     avail_region_offset < num_available_memory_regions;
	     ++avail_region_offset) {
		try_add_available_memory_region(
			available_memory_regions[avail_region_offset],
			try_add_available_memory_region);
	}
	/* Avoid walking off the start of the chain */
	first_header = &first_chunk->headers[0];
	first_header[0].prev = nullptr;
	modifying_pmm.release_lock();
	/* And reserve the location the headers are currently at */
	if (get_mem_area(phys_hdr_ptr, headers_needed_size) != pmm_success) {
		kcriticalf("Unable to reserve the PMMs memory at %p (mapped to %p)!",
		           phys_hdr_ptr.unsafe_raw_get(), first_chunk);
	} else {
		klogf("Reserved space for the PMM at %p (mapped to %p)",
		      phys_hdr_ptr.unsafe_raw_get(), first_chunk);
	}
	return 0;
}

static PhysMemHeaderList *get_header_chunk();

/* Get the next available header (returns nullptr if none are available) */
static PhysMemHeader *get_new_header() {
	PhysMemHeaderList *chunk = first_chunk;

	/* Preemptively grab a new header list if there is only one header
	 * available (because we need one to allocate the list). Don't reset chunk
	 * to first_chunk, because by definition it must be the first place we can
	 * allocate a new header. If get_header_chunk, it returns null, which causes
	 * us to transparently skip the while loop, and return nullptr
	 */
	assert(headers_in_use <= headers_allocated);
	if (headers_in_use + 1 >= headers_allocated) {
		while (chunk->next != nullptr) {
			chunk = chunk->next;
		}
		chunk->next = get_header_chunk();
		chunk = chunk->next;
	}

	/* Go through each chunk of headers */
	while (chunk != nullptr) {
		/* And iterate through each header in that chunk */
		for (auto &header : chunk->headers) {
			/* Returning the header if we can use it */
			if (!header.header_in_use) {
				assert(header.canary[0] == 0xCD);
				assert(header.canary[1] == 0xEF);
				/* Marking it as used now */
				header.header_in_use = true;
				return &header;
			}
		}
		/* None of the headers were available, go to the next chunk */
		chunk = chunk->next;
	}
	/* None of the chunks had an available header */
	kerror("Unable to get a new header for the PMM to use! System will likely "
	       "crash soon!");
	klogf("PMM headers: %#jx available/%#jx allocated!", headers_in_use,
	      headers_allocated);
	return nullptr;
}

enum allocation_strategy {
	first_fit,
};

/* Find num unused (contiguous) pages */
/* modifying_pmm must be locked before calling this! */
/* returns nullptr if no RAM is available */
static PhysMemHeader *find_free_pages(page len, allocation_strategy strategy
                                      [[maybe_unused]]) {
	for (PhysMemHeader *cur_header = first_header; cur_header != nullptr;
	     cur_header = cur_header->next) {
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
static PhysMemHeader *find_header_for_page(PhysAddr<void const> const addr,
                                           size_t len) {
	for (PhysMemHeader *cur_header = first_header; cur_header != nullptr;
	     cur_header = cur_header->next) {
		assert(cur_header->header_in_use);
		if (cur_header->memory.getInt() <= addr.as_int() &&
		    cur_header->memory.getInt() + cur_header->size.getInt() >=
		        addr.as_int()) {
			/* We found the header, now we just need to check if we can use it
			 */
			if (cur_header->in_use || cur_header->size.getInt() < len) {
				kwarnf(
					"Header for memory at %p already in use. Not returning it.",
					addr.unsafe_raw_get());
				return nullptr;
			} else {
				return cur_header;
			}
		}
	}
	/* We couldn't find the header */
	return nullptr;
}

/* Make the header point to only len amount of memory if possible */
/* No guarantees, because allocating a new header might be necessary and fail */
/* However, if it doesn't work, everything is valid, just suboptimal */
static void split_header_at_len(PhysMemHeader &header, page len) {
	assert(header.header_in_use);
	assert(header.in_use);
	assert(header.canary[0] == 0xCD);
	assert(header.canary[1] == 0xEF);
	assert(header.size >= len);
	if (header.size == len) {
		return;
	}
	/* Get a new header to hold the split */
	PhysMemHeader *temp = get_new_header();
	/* We didn't get a new header, so don't split, just return extra memory */
	/* This might be important if we're reserving more memory for a new chunk */
	if (temp == nullptr) {
		kwarn(
			"Unable to get a new header for the PMM! Returning extra memory.");
		return;
	}
	assert(temp->canary[0] == 0xCD);
	assert(temp->canary[1] == 0xEF);
	/* Initialize the new header */
	*temp = PhysMemHeader{.next = header.next,
	                      .prev = &header,
	                      .memory = header.memory + len,
	                      .size = header.size - len,
	                      .in_use = false,
	                      .canary = {0xCD, 0xEF},
	                      .header_in_use = true};
	headers_in_use += 1;
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
	assert(header.canary[0] == 0xCD);
	assert(header.canary[1] == 0xEF);
	header.in_use = true;
	return pmm_success;
}

static PhysMemHeaderList *get_header_chunk() {
	klog("Allocating new header chunk!");
	PhysMemHeader *header = find_free_pages(1, first_fit);
	if (!header) {
		kerror("No free pages for new header available!");
		return nullptr;
	}
	if (internal_claim_mem_area(*header) != pmm_success) {
		return nullptr;
	}
	PhysMemHeaderList *new_chunk;
	map_results results = map_range(PhysAddr<void>(header->memory.getInt()),
	                                sizeof(PhysMemHeaderList),
	                                reinterpret_cast<void **>(&new_chunk), 0);
	if (results != map_success) {
		/* Unclaim the header if mapping failed */
		kerror("Mapping chunk for new header failed!");
		header->in_use = false;
		return nullptr;
	} else {
		for (auto &hdr : *new_chunk) {
			hdr = {.next = nullptr,
			       .prev = nullptr,
			       .memory = nullptr,
			       .size = nullptr,
			       .in_use = true,
			       .canary = {0xCD, 0xEF},
			       .header_in_use = false};
		}
		headers_allocated += new_chunk->size();
		return new_chunk;
	}
}

/* Reserve len unused bytes from addr (if available) */
pmm_results get_mem_area(PhysAddr<void const> const addr, uintptr_t len) {
	modifying_pmm.acquire_lock();
	PhysMemHeader *header = find_header_for_page(addr, len);
	if (!header) {
		modifying_pmm.release_lock();
		return pmm_nomem;
	}
	pmm_results temp = internal_claim_mem_area(*header);
	if (temp == pmm_success) {
		/* Remove stuff from before it */
		if (header->memory > addr.unsafe_raw_get()) {
			split_header_at_len(*header,
			                    (addr - header->memory.getInt()).as_int());
			header->in_use = false;
			header = header->next;
			internal_claim_mem_area(*header);
		}
		split_header_at_len(*header, round_up_to_page(len));
	}
	modifying_pmm.release_lock();
	return temp;
}

/* Acquire len unused bytes */
pmm_results get_mem_area(PhysAddr<void const> *addr, uintptr_t len) {
	modifying_pmm.acquire_lock();
	auto *header = find_free_pages(round_up_to_page(len), first_fit);
	if (header == nullptr) {
		modifying_pmm.release_lock();
		return pmm_nomem;
	}
	pmm_results result = internal_claim_mem_area(*header);
	if (result == pmm_success) {
		split_header_at_len(*header, round_up_to_page(len));
		*addr = PhysAddr<void>(header->memory.getInt());
	}
	modifying_pmm.release_lock();
	return result;
}

/* Merge the headers next to this one if they are both free */
static void merge_adjacent_headers(PhysMemHeader &header) {
	if (header.next && !header.next->in_use &&
	    header.memory + header.size == header.next->memory) {
		header.next->header_in_use = false;
		header.size += header.next->size;
		if (header.next->next) {
			header.next->next->prev = &header;
		}
		header.next = header.next->next;
		headers_in_use -= 1;
	}
	if (header.prev && !header.prev->in_use &&
	    header.prev->memory + header.prev->size == header.memory) {
		header.header_in_use = false;
		header.prev->size += header.size;
		header.prev->next = header.next;
		if (header.next) {
			header.next->prev = header.prev;
		}
		headers_in_use -= 1;
	}
}

/* Free len bytes from addr */
pmm_results free_mem_area(PhysAddr<void const> const addr, uintptr_t len) {
	modifying_pmm.acquire_lock();
	for (PhysMemHeader *cur_header = first_header; cur_header != nullptr;
	     cur_header = cur_header->next) {
		assert(cur_header->header_in_use);
		if (cur_header->memory.getInt() <= addr.as_int() &&
		    cur_header->memory.getInt() + cur_header->size.getInt() >
		        addr.as_int()) {
			auto end = addr + len;
			if (cur_header->memory.getInt() + cur_header->size.getInt() <
			    end.as_int()) {
				/* We only know about part of the area? */
				kwarnf("Attempt to free area %p-%p, but header only covers "
				       "area %p-%p!",
				       addr.unsafe_raw_get(), end.unsafe_raw_get(),
				       cur_header->memory.get(),
				       (cur_header->memory + cur_header->size).get());
				/* TODO: should we abort? */
				std::abort();
			}
			if (!cur_header->in_use) {
				modifying_pmm.release_lock();
				return pmm_invalid;
			}
			cur_header->in_use = false;
			merge_adjacent_headers(*cur_header);
			modifying_pmm.release_lock();
			return pmm_success;
		}
	}
	modifying_pmm.release_lock();
	/* We never found the header, so it was an invalid free */
	kerrorf("Failing to free %p", addr.unsafe_raw_get());
	dump_pagetables();
	return pmm_invalid;
}

void ensure_not_allocatable(PhysAddr<void> addr, size_t len) {
	auto search_range =
		range{.start = addr.as_int(), .end = (addr + len).as_int()};
	for (PhysMemHeader *cur_header = first_header; cur_header;
	     cur_header = cur_header->next) {
		auto target_range =
			range{.start = cur_header->memory.getInt(),
		          .end = (cur_header->memory + cur_header->size).getInt()};
		if (overlap(search_range, target_range)) {
			/* If the search range completely covers the target range,
			 * completely remove the target range. */
			if (search_range.start <= target_range.start &&
			    search_range.end >= target_range.end) {
				klog("Clearing full header");
				cur_header->in_use = false;
				if (cur_header->next) {
					cur_header->next->prev = cur_header->prev;
				}
				if (cur_header->prev) {
					cur_header->prev->next = cur_header->next;
				}
				cur_header->header_in_use = false;
			} else if (search_range.start <= target_range.start) {
				klog("Shrinking header");
				cur_header->memory = search_range.end;
				cur_header->size = target_range.end - search_range.end;
			} else if (search_range.end >= target_range.end) {
				klog("Shrinking header");
				cur_header->size = search_range.start - target_range.start;
			} else {
				klogf("Need to split range!");
				std::abort();
			}
		}
	}
}

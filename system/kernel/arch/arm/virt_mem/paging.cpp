/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2023 James McNaughton Felder */
#include <kernel/vtopmem.h>
#include <kernel/paging.h>
#include <kernel/mem.h>
#include <kernel/log.h>
#include <cstdlib>
#include <cassert>
#include <cstring>
#include <feline/spinlock.h>

/* Begin Private Declarations */

/* Return the offset into a page */
inline uintptr_t page_offset(uintptr_t const addr){
	return addr&(PHYS_MEM_CHUNK_SIZE-1);
}
inline uintptr_t page_offset(void const * const addr){
	return page_offset(reinterpret_cast<uintptr_t>(addr));
}

/* Return the index of the searchable page tables for addr */
inline size_t searchable_page_table_offset(page const addr){
	return addr.getInt()/PHYS_MEM_CHUNK_SIZE;
}

/* Check if a virtual address is mapped */
/* NOTE: this is the truth, ignoring any kernel tricks */
/* Don't call this function if you haven't locked `modifying_page_tables` */
bool isMapped(page const virt_addr);

/* Check if needed pages are free starting at virt_addr_base */
/* Don't call this function if you haven't locked `modifying_page_tables` */
bool free_from_here(page virt_addr_base, uintptr_t needed);
bool free_from_here(page *virt_addr_base, uintptr_t needed);

/* Find len bytes of unmapped memory */
/* Don't call this function if you haven't locked `modifying_page_tables` */
void *find_free_virtmem(uintptr_t len);

/* Map phys_addr to virt_addr */
/* Don't call this function if you haven't locked `modifying_page_tables` */
map_results map_page(page const phys_addr, page const virt_addr, unsigned int opts);
/* Unmap the page containing virt_addr */
/* Don't call this function if you haven't locked `modifying_page_tables` */
map_results unmap_page(page const virt_addr, unsigned int opts);

/* End Private Declarations */

/* Begin Global Variables */

/* Lock this before modifying any page tables */
Spinlock modifying_page_tables;

/* Only take the address of these! */
extern char const phys_kernel_start;
extern char const phys_kernel_end;
extern char const kernel_start;
extern char const kernel_end;

/* Maximuim amount of virtual memory */
/* Change for 64-bit */
#define MAX_VIRT_MEM 4_GiB

/* What we use for searching */
/* True if present, false otherwise */
/* Keep in sync with the CPU's page tables! */
bool page_tables_searchable[MAX_VIRT_MEM/PHYS_MEM_CHUNK_SIZE]={false};

/* End Global Variables */

/* Don't call this function if you haven't locked `modifying_page_tables` */
bool isMapped(page const virt_addr){
	/* If the page table is present */
	return page_tables_searchable[searchable_page_table_offset(virt_addr)];
}

/* Check if needed pages are free starting at virt_addr_base */
/* If they aren't, virt_addr_base is updated to the first in-use page greater than what it was */
/*	or 0 if it would wrap around */
/* lock modifying_page_tables before calling this */
bool free_from_here(page *virt_addr_base, uintptr_t needed){
	/* If we would overflow */
	/* read as (virt_addr_base->getInt()+needed >= MAX_VIRT_MEM) that actually checks for overflow */
	if(virt_addr_base->getInt()>MAX_VIRT_MEM-needed){
		/* return an error */
		virt_addr_base->set(nullptr);
		return false;
	}
	page last_page_needed=virt_addr_base->getInt()+needed;
	/* Round up if needed */
	if (page_offset(needed) != 0) {
		++last_page_needed;
	}
	/* For each page that would be needed */
	for(page base=*virt_addr_base; base<=last_page_needed; base++) {
		if(isMapped(base)) {
			virt_addr_base->set(base);
			return false;
		}
	}
	return true;
}
/* Same as above, except doesn't modify virt_addr_base */
bool free_from_here(page virt_addr_base, uintptr_t needed){
	return free_from_here(&virt_addr_base, needed);
}

/* Find len bytes of unmapped memory */
void *find_free_virtmem(uintptr_t len){
	/* Loop through all of the virtual memory */
	for(page base=4_KiB; base.getInt()<(MAX_VIRT_MEM-PHYS_MEM_CHUNK_SIZE) && !base.isNull(); base++){
		/* Check if enough memory is free */
		if(free_from_here(&base, len)){
			/* returns true if base points to len bytes of free memory */
			return base;
		}
		/* If it set base to null, we are out of room */
		if(base.isNull()){
			kwarn("No memory left!");
			return nullptr;
		}
		/* free_from_here() moves base to blocked memory if it isn't at the start of enough free memory */
	}
	return nullptr;
}

/* Map virt_addr to phys_addr (rounding both down to multiple of 4KiB) */
/* Don't call this function if you haven't locked `modifying_page_tables` */
map_results map_page(page const phys_addr, page const virt_addr, unsigned int opts[[maybe_unused]]){
	if (phys_addr==virt_addr) {
		invlpg(virt_addr);
		page_tables_searchable[searchable_page_table_offset(virt_addr)]=true;
		return map_success;
	}
	else {
		return map_invalid_align;
	}
}

/* Map len bytes from phys_addr to virt_addr (internal use only) */
/* Don't call this function if you haven't locked `modifying_page_tables` */
static map_results internal_map_range(void const * const phys_addr, uintptr_t len, void const * const virt_addr, unsigned int opts){
	/* Verify correct alignment */
	if (page_offset(phys_addr) != page_offset(virt_addr)){
		return map_invalid_align;
	}
	/* Create the page objects for internal use */
	page virt_to_map=virt_addr;
	page phys_to_map=phys_addr;
	/* If we can map it */
	if((opts & MAP_OVERWRITE) != 0 || free_from_here(virt_to_map, len)){
		/* Figure out how many pages we need to map */
		/* The only way this doesn't work is if we round down to get the base page */
		/*	more than this rounds up to get an amount of pages */
		/*	since this only results in an extra page being mapped, don't worry about it for now */
		size_t numPages=bytes_to_pages(len);
		/* Create this outside of the for loop */
		map_results attempt;
		/* Loop through the right amount of pages, incrementing everything */
		for(size_t count=0; count<numPages; count++, virt_to_map++, phys_to_map++){
			/* Attempt to map the page (we already have the lock) */
			attempt=map_page(phys_to_map, virt_to_map, opts);
			/* If it failed */
			/* NOTE: getting here is a bug (is the spinlock not working?) */
			if(attempt!=map_success){
				kerrorf("Mapping a page failed with error %d. Is the spinlock working?", attempt);
				/* Loop until we get back to where we started */
				while(virt_to_map>virt_addr){
					/* Decrementing first means we don't unmap the failed map and that we unmap the first mapping */
					virt_to_map--;
					/* Unmap the page */
					if(unmap_page(virt_to_map, 0)!=map_success){
						/* The only reason I can think for this to fail is if the spinlock is broken and someone else has: */
						/*	a: mapped something where we are trying to and */
						/*	b: unmapped one of our previously mapped pages */
						/* So it is safe to say our code isn't working and someone is trying to cause a crash. */
						/* Abort should be pretty safe */
						abort();
					}
				}
				/* Propegate the error out */
				return attempt;
			}
		}
		/* And everything is complete */
		return map_success;
	}
	/* And we couldn't map everything */
	return map_already_mapped;
}

/* Mapping a range with phys_addr and virt_addr specified */
map_results map_range(void const * const phys_addr, uintptr_t len, void const * const virt_addr, unsigned int opts){
	/* Synchronize access */
	modifying_page_tables.aquire_lock();
	map_results temp=internal_map_range(phys_addr, len, virt_addr, opts);
	modifying_page_tables.release_lock();
	return temp;
}

/* Mapping a range with only phys_addr specified */
map_results map_range(void const * const phys_addr, uintptr_t len, void **virt_addr, unsigned int opts){
	modifying_page_tables.aquire_lock();
	/* Round up, instead of down. */
	len += page_offset(phys_addr);
	if (free_from_here(phys_addr, len)) {
		*virt_addr=const_cast<void*>(phys_addr);
	}
	else {
		virt_addr=nullptr;
	}
	// *virt_addr=find_free_virtmem(len);
	if(*virt_addr==nullptr){
		return map_no_virtmem;
	}
	/* Set it to the correct offset in the page */
	map_results temp=internal_map_range(phys_addr, len, *virt_addr, opts);
	// *virt_addr = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(*virt_addr) + page_offset(phys_addr));
	modifying_page_tables.release_lock();
	return temp;
}

/* Mapping a range with only virt_addr specified */
map_results map_range(uintptr_t len, void const * const virt_addr, unsigned int opts){
	modifying_page_tables.aquire_lock();
	void *phys_addr;
	/* attempt to get the physical memory */
	pmm_results attempt=get_mem_area(&phys_addr, len, 0);
	/* if we are out */
	if(attempt==pmm_nomem){
		modifying_page_tables.release_lock();
		/* return the error */
		return map_no_physmem;
	}
	/* Set it to the correct offset in the page */
	phys_addr = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(phys_addr) + page_offset(virt_addr));
	map_results temp;
	temp=internal_map_range(phys_addr, len, virt_addr, opts);
	modifying_page_tables.release_lock();
	return temp;
}

/* Mapping a range with nothing specified */
map_results map_range(uintptr_t len, void **virt_addr, unsigned int opts){
	modifying_page_tables.aquire_lock();
	*virt_addr=find_free_virtmem(len);
	if(*virt_addr==nullptr){
		return map_no_virtmem;
	}
	void *phys_addr;
	/* attempt to get the physical memory */
	pmm_results attempt=get_mem_area(&phys_addr, len, 0);
	/* if we are out */
	if(attempt==pmm_nomem){
		modifying_page_tables.release_lock();
		/* return the error */
		return map_no_physmem;
	}
	map_results temp;
	temp=internal_map_range(phys_addr, len, virt_addr, opts);
	modifying_page_tables.release_lock();
	return temp;
}

/* TODO; add many checks to avoid unmapping or leaking info about the kernel */
map_results unmap_page(page const virt_addr, unsigned int opts [[maybe_unused]]){
	if(isMapped(virt_addr)){
		page_tables_searchable[searchable_page_table_offset(virt_addr)]=false;
		/* Invalidate the cpu's cache */
		invlpg(virt_addr);
		return map_success;
	}
	else{
		return map_notmapped;
	}
}

map_results unmap_range(void const * const virt_addr, uintptr_t len, unsigned int opts [[maybe_unused]]){
	modifying_page_tables.aquire_lock();
	/* Loop through */
	page to_unmap=virt_addr;
	for(size_t count=0; count<bytes_to_pages(len); count++, to_unmap++){
		/* If anything isn't mapped */
		if(!isMapped(to_unmap)){
			/* Return the error */
			modifying_page_tables.release_lock();
			return map_notmapped;
		}
	}
	/* If we are managing the physical memory */
	if((opts & PHYS_ADDR_AUTO) != 0){
		/* We can't do this until we actually keep track of what the physical memory is
		 * Will come when we have an actual implimentation */
		return map_invalid_option;
		/* Attempt to free it */
		// pmm_results attempt=free_mem_area(virt_to_phys(virt_addr), len, 0);
		/* If the attempt failed */
		// if(attempt==pmm_invalid || attempt==pmm_null){
			/* Call it an invalid option because we shouldn't have been managing it(TODO: better description) */
			// return map_invalid_option;
		// }
	}
	/* Loop through again */
	to_unmap=virt_addr;
	for(size_t count=0; count<bytes_to_pages(len); count++, to_unmap++){
		/* Actually unmap it */
		unmap_page(to_unmap, 0);
	}
	modifying_page_tables.release_lock();
	return map_success;
}

int setup_paging(){
	/* Map the kernel and page directory */
	/* This makes sure that they haven't been unmapped */
	modifying_page_tables.aquire_lock();
	/* Basic sanity check */
	/* If this fails, we probably would crash on the instruction after enabling paging */
	// assert(isMapped(&kernel_start));
	// assert(isMapped(&kernel_end));
	modifying_page_tables.release_lock();
	return 0;
}

/* Copied from https://wiki.osdev.org/Paging#INVLPG */
void invlpg(page const addr [[maybe_unused]]){
}

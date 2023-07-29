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

/* Bits (MSB to LSB): addr: 20, unused: 4, mb_page: 1, written: 1, read: 1, */
/*	cache_disable: 1, write_through: 1, user: 1, writeable: 1, present: 1 */
typedef uint32_t page_directory_entry;

/* Bits (MSB to LSB): addr: 20, unused: 3, global: 1, mb_page: 1, written: 1, read: 1, */
/*	cache_disable: 1, write_through: 1, user: 1, writeable: 1, present: 1 */
typedef uint32_t mb_page;

/* Bits (MSB to LSB): addr: 20, unused: 3, global: 1, pat: 1, written: 1, read: 1, */
/*	cache_disable: 1, write_through: 1, user: 1, writeable: 1, present: 1 */
typedef uint32_t page_table_entry;

/* Bits in page tables/directories */
#define PRESENT			0b1
#define WRITEABLE		0b10
#define USER			0b100
#define WRITE_THROUGH	0b1000
#define CACHE_DISABLE	0b10000
#define READ			0b100000
#define WRITTEN			0b1000000
/* Is it a page directory or 2MB page */
/* not useable with PAT */
#define MB_PAGE			0b10000000
/* Does the 2KiB page table have a special memory type */
/* not useable with MB_PAGE */
#define PAT				0b10000000
/* Is the 4KiB page table a global page */
#define GLOBAL			0b100000000

/* Begin Private Declarations */

/* Return the offset into a page */
inline uintptr_t page_offset(uintptr_t const addr){
	return addr&(PHYS_MEM_CHUNK_SIZE-1);
}
inline uintptr_t page_offset(void const * const addr){
	return page_offset(reinterpret_cast<uintptr_t>(addr));
}

/* Set a bit in a page table/directory */
inline int set_bit(page_table_entry * const p, uint32_t const b){
	*p|=b;
	return 0;
}
/* Unset a bit in a page table/directory */
inline int unset_bit(page_table_entry * const p, uint32_t const b){
	*p&=~b;
	return 0;
}
/* Check if a bit is set in a page table/directory */
inline bool is_set(page_table_entry const p, uint32_t const b){
	return (p & b) != 0;
}
inline bool is_set(page_table_entry const *p, uint32_t const b){
	return (*p & b) != 0;
}

/* Is the page table/directory present */
constexpr inline bool present(page_table_entry const p){
	return (p & PRESENT) != 0;
}
constexpr inline bool present(page_table_entry const * const p){
	return (*p & PRESENT) != 0;
}

/* What is the address pointed to by the page table/directory */
constexpr inline uint32_t addr(page_table_entry const p){
	return p & ~0xfff_uint32_t;
}
constexpr inline uint32_t addr(page_table_entry const * const p){
	return *p & ~0xfff_uint32_t;
}
/* Set the address a page table/directory points to */
inline void set_addr(page_table_entry * const p, uint32_t const addr){
	*p&=0xfff;
	*p|=addr&~0xfff_uint32_t;
}
inline void set_addr(page_table_entry * const p, void const * const addr){
	return set_addr(p, reinterpret_cast<uintptr_t>(addr));
}

/* Malloced array of 1024 elements */
page_table_entry* create_new_page_table();

/* Return the offset into the page table you need to go for addr */
inline uintptr_t pte_offset(page const addr){
	return addr.getInt() >> 12 & 0x03ff_uint32_t;
}

/* Return the offset into the page directory you need to go for addr */
inline uintptr_t pde_offset(page const addr){
	return addr.getInt()>>22;
}

/* Return the index of the searchable page tables for addr */
inline size_t searchable_page_table_offset(page const addr){
	return addr.getInt()/PHYS_MEM_CHUNK_SIZE;
}

/* Turn an address into something that can be ANDed with a page table entry */
inline page_table_entry *addr2pte(page const addr){
	return reinterpret_cast<page_table_entry*>(addr.getInt()<<12);
}

/* Get the address a page table entry is pointing to */
inline page pt_addr2addr(uint32_t const addr){
	return page(addr & ~0xFFFull);
}

/* Check if a virtual address is mapped */
/* NOTE: this is the truth, ignoring any kernel tricks */
/* Don't call this function if you haven't locked `modifying_page_tables` */
bool isMapped(page const virt_addr);

/* Returns the virtual address pd[pdindex]->pt[ptindex]+offset(phys_addr) controls */
inline void* get_virt_addr(unsigned int const pdindex, unsigned int const ptindex, void const * const phys_addr){
	return reinterpret_cast<void*>(
		pdindex*4096_KiB+ptindex*4_KiB+
		(reinterpret_cast<uintptr_t>(phys_addr) & 0xfff)
	);
}

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

/* Mark a page table or directory not present */
void mark_notpresent(page_directory_entry *pd);
void mark_notpresent(page_table_entry *pt);

/* get the physical address virt_addr points to */
/* for now returns a valid address even if the page table isn't "present" */
inline void *virt_to_phys(void const * const virt_addr){
	return reinterpret_cast<void*>(addr(reinterpret_cast<page_table_entry*>(0xFFC00000 + (1024*sizeof(page_table_entry)*pde_offset(virt_addr)) + (sizeof(page_table_entry)*pte_offset(virt_addr)))));

}

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

/* Initialize immediately */
/* What the CPU sees */
page_directory_entry bootstrap_page_directory[[gnu::aligned(0x1000)]][1024]={0};
page_directory_entry *page_directory=bootstrap_page_directory;
page_table_entry all_page_tables[[gnu::aligned(0x1000)]][1024][1024]={{0}};
page_table_entry *page_tables=*all_page_tables;
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

/* Not used, but keep for when we are doing stuff dynamically */
page_table_entry* create_new_page_table(){
	void *marea;
	get_mem_area(&marea, PHYS_MEM_CHUNK_SIZE, 0);
	std::memset(marea, 0, sizeof(page_table_entry)*1024);
	return static_cast<page_table_entry*>(marea);
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
map_results map_page(page const phys_addr, page const virt_addr, unsigned int opts){
	if((opts & MAP_OVERWRITE) == 0 && isMapped(virt_addr)){
		/* TODO: check that we aren't leaking info about the kernel */
		return map_already_mapped;
	}
	/* Get the page directory we are working with */
	page_directory_entry *cur_pde=reinterpret_cast<page_directory_entry*>(0xFFFFF000 + sizeof(page_directory_entry)*pde_offset(virt_addr));
	/* If it isn't present */
	if(!present(cur_pde)){
		/* Create a new page table and save the address */
		/* Not using now because everything is statically set */
		/* set_addr(cur_pde, create_new_page_table() - VA_OFFSET); */
		set_addr(cur_pde, reinterpret_cast<uintptr_t>(&all_page_tables[pde_offset(virt_addr)]) - VA_OFFSET);
		/* Set the configuration */
		unset_bit(cur_pde, GLOBAL);
		unset_bit(cur_pde, MB_PAGE); /* Page directory, not 4MB page table */
		unset_bit(cur_pde, WRITTEN); /* Not accessed yet */
		unset_bit(cur_pde, READ);
		unset_bit(cur_pde, CACHE_DISABLE); /* Cache this */
		unset_bit(cur_pde, WRITE_THROUGH); /* Write-back caching */
		unset_bit(cur_pde, USER); /* Default kernelspace */
		set_bit(cur_pde, WRITEABLE); /* Default writeable */
		set_bit(cur_pde, PRESENT); /* It is useable */
	}
	page_directory_entry *cur_pte=reinterpret_cast<page_table_entry*>(0xFFC00000 + (1024*sizeof(page_table_entry)*pde_offset(virt_addr)) + (sizeof(page_table_entry)*pte_offset(virt_addr)));
	set_addr(cur_pte, phys_addr);
	unset_bit(cur_pte, GLOBAL); /* Default process local */
	unset_bit(cur_pte, PAT); /* We don't support PAT for memory types (yet) */
	unset_bit(cur_pte, WRITTEN); /* Not accessed yet */
	unset_bit(cur_pte, READ);
	unset_bit(cur_pte, CACHE_DISABLE); /* Cache this */
	unset_bit(cur_pte, WRITE_THROUGH); /* Write-back caching */
	unset_bit(cur_pte, USER); /* Default kernelspace */
	set_bit(cur_pte, WRITEABLE); /* Default writeable */
	set_bit(cur_pte, PRESENT); /* It is useable */
	page_tables_searchable[searchable_page_table_offset(virt_addr)]=true;
	assert(isMapped(virt_addr));
	invlpg(virt_addr);
	return map_success;
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
	*virt_addr=find_free_virtmem(len);
	if(*virt_addr==nullptr){
		return map_no_virtmem;
	}
	/* Set it to the correct offset in the page */
	*virt_addr = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(*virt_addr) + page_offset(phys_addr));
	map_results temp=internal_map_range(phys_addr, len, *virt_addr, opts);
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
		unset_bit(reinterpret_cast<page_table_entry*>(0xFFC00000 + (1024*sizeof(page_table_entry)*pde_offset(virt_addr)) + (sizeof(page_table_entry)*pte_offset(virt_addr))), PRESENT);
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
		/* Attempt to free it */
		pmm_results attempt=free_mem_area(virt_to_phys(virt_addr), len, 0);
		/* If the attempt failed */
		if(attempt==pmm_invalid || attempt==pmm_null){
			/* Call it an invalid option because we shouldn't have been managing it(TODO: better description) */
			return map_invalid_option;
		}
	}
	/* Loop through again */
	to_unmap=virt_addr;
	size_t searchable_offset=searchable_page_table_offset(virt_addr);
	for(size_t count=0; count<bytes_to_pages(len); count++, to_unmap++){
		/* Actually unmap it */
		unmap_page(to_unmap, 0);
		page_tables_searchable[searchable_offset+count]=false;
	}
	modifying_page_tables.release_lock();
	return map_success;
}

void mark_notpresent(page_table_entry *pt){
	unset_bit(pt, PRESENT);
	return;
}

int setup_paging(){
	uintptr_t cr3=reinterpret_cast<uintptr_t>(bootstrap_page_directory) - VA_OFFSET;
	/* Lower bits need to be zero because it needs to be aligned */
	assert((cr3 & 0xFFF) == 0);

	for(size_t pdindex=0; pdindex<1024; pdindex++){
		/* Setup the page directory */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpragmas" //GCC doesn't know the next warning, but Clang does and needs it suppressed with -Werror
#pragma GCC diagnostic ignored "-Warray-bounds-pointer-arithmetic" //Converting virtual to physical memory will require going outside of the array
		set_addr(&bootstrap_page_directory[pdindex], all_page_tables[pdindex] - VA_OFFSET/sizeof(page_table_entry));
#pragma GCC diagnostic pop
		unset_bit(&bootstrap_page_directory[pdindex], MB_PAGE); /* Page directory, not 4MB page table */
		unset_bit(&bootstrap_page_directory[pdindex], WRITTEN); /* Not accessed yet */
		unset_bit(&bootstrap_page_directory[pdindex], READ);
		unset_bit(&bootstrap_page_directory[pdindex], CACHE_DISABLE); /* Cache this */
		unset_bit(&bootstrap_page_directory[pdindex], WRITE_THROUGH); /* Write-back caching */
		unset_bit(&bootstrap_page_directory[pdindex], USER); /* Default kernelspace */
		set_bit(&bootstrap_page_directory[pdindex], WRITEABLE); /* Default writeable */
		set_bit(&bootstrap_page_directory[pdindex], PRESENT); /* It is useable */
		/* The page table is statically set to 0, don't waste CPU time */
		/* The searchable page table is statically set to 0, don't waste CPU time */
	}

	/* Map the kernel and page directory */
	/* This makes sure that they haven't been unmapped */
	modifying_page_tables.aquire_lock();
	for(page where=&kernel_start; where <= &kernel_end; ++where) {
		page phys_where=where.getInt() - VA_OFFSET;
		page_table_entry *cur_pte=&all_page_tables[pde_offset(where)][pte_offset(where)];
		set_addr(cur_pte, phys_where);
		unset_bit(cur_pte, GLOBAL); /* Default process local */
		unset_bit(cur_pte, PAT); /* We don't support PAT for memory types (yet) */
		unset_bit(cur_pte, WRITTEN); /* Not accessed yet */
		unset_bit(cur_pte, READ);
		unset_bit(cur_pte, CACHE_DISABLE); /* Cache this */
		unset_bit(cur_pte, WRITE_THROUGH); /* Write-back caching */
		unset_bit(cur_pte, USER); /* Default kernelspace */
		set_bit(cur_pte, WRITEABLE); /* Default writeable */
		set_bit(cur_pte, PRESENT); /* It is useable */
		page_tables_searchable[searchable_page_table_offset(where)]=true;
		assert(isMapped(where));
		invlpg(where);
	}
	set_addr(&bootstrap_page_directory[1023], reinterpret_cast<uintptr_t>(bootstrap_page_directory) - VA_OFFSET);
	unset_bit(&bootstrap_page_directory[1023], GLOBAL);
	unset_bit(&bootstrap_page_directory[1023], PAT);
	unset_bit(&bootstrap_page_directory[1023], GLOBAL);
	unset_bit(&bootstrap_page_directory[1023], WRITTEN);
	unset_bit(&bootstrap_page_directory[1023], READ);
	unset_bit(&bootstrap_page_directory[1023], CACHE_DISABLE);
	unset_bit(&bootstrap_page_directory[1023], WRITE_THROUGH);
	unset_bit(&bootstrap_page_directory[1023], USER);
	set_bit(&bootstrap_page_directory[1023], WRITEABLE);
	set_bit(&bootstrap_page_directory[1023], PRESENT);
	/* Mark the recursive page table mapping as used */
	memset(&page_tables_searchable[0xffc00], true, sizeof(*page_tables_searchable)*(0x100000-0xffc00));
	/* Basic sanity check */
	/* If this fails, we probably would crash on the instruction after enabling paging */
	assert(isMapped(&kernel_start));
	assert(isMapped(&kernel_end));
	modifying_page_tables.release_lock();
	/* Writeback instead of writethrough (PWT==1<<3) */
	/* cr3 |= (1<<3); */
	/* Don't disable caching (PCD==1<<4) */
	/* cr3 |= (1<<4); */
	enable_paging(cr3);
	/* Change where we can access the page tables */
	page_directory=reinterpret_cast<page_directory_entry*>(0xffc00000);
	return 0;
}

/* Copied from https://wiki.osdev.org/Paging#INVLPG */
void invlpg(page const addr){
	asm volatile("invlpg (%0)" :: "b" (addr.get()) : "memory");
}

/*
 * Print the paging tables similar to how Bochs does it
 * It stops before the paging tables, because otherwise the output is crazy long
 *   (which is what bochs does)
 * If you need to see it, change the 0xffc00000 in the loop test to UINTPTR_MAX
 *   and redirect to a file.
 */
void dump_pagetables() {
	page prev_phys_addr=nullptr;
	page prev_virt_addr=nullptr;
	page contiguous_phys_addr_start=nullptr;
	page contiguous_virt_addr_start=nullptr;
	size_t num_contiguous_mappings=0;
	for (page virt_addr=nullptr; virt_addr.getInt() != 0xffc00000; ++virt_addr) {
		page phys_addr=pt_addr2addr(all_page_tables[pde_offset(virt_addr)][pte_offset(virt_addr)]);
		bool is_jump =
			!isMapped(virt_addr) ||
			!isMapped(prev_virt_addr)
			|| prev_phys_addr+page{PHYS_MEM_CHUNK_SIZE} != phys_addr;
		if (is_jump) {
			if (num_contiguous_mappings > 0) {
				printf("%p-%p -> %p-%p (%#llx)\n",
						contiguous_virt_addr_start.get(), reinterpret_cast<void*>(prev_virt_addr.getInt()+0xfff),
						contiguous_phys_addr_start.get(), reinterpret_cast<void*>(prev_phys_addr.getInt()+0xfff),
						num_contiguous_mappings*PHYS_MEM_CHUNK_SIZE
					  );
			}
			if (isMapped(virt_addr)) {
				num_contiguous_mappings=1;
			}
			else {
				num_contiguous_mappings=0;
			}
			contiguous_virt_addr_start=virt_addr;
			contiguous_phys_addr_start=phys_addr;
		}
		else {
			++num_contiguous_mappings;
		}
		prev_virt_addr=virt_addr;
		prev_phys_addr=phys_addr;
	}
	if (num_contiguous_mappings != 0) {
		printf("%p-%p -> %p-%p (%#llx)\n",
				contiguous_virt_addr_start.get(), reinterpret_cast<void*>(prev_virt_addr.getInt()+0xfff),
				contiguous_phys_addr_start.get(), reinterpret_cast<void*>(prev_phys_addr.getInt()+0xfff),
				num_contiguous_mappings*PHYS_MEM_CHUNK_SIZE
			  );
	}
}

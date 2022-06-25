// SPDX-License-Identifier: MIT
// Copyright (c) 2021 James McNaughton Felder
#include "mem.h"
#include <cassert>
#include <kernel/log.h>
#include <kernel/paging.h>
#include <cstring>
#include <cstdlib>
#include <cinttypes>
#include <feline/fixed_width.h>
#include <feline/spinlock.h>
#include <feline/bool_int.h>

//An array of strings for all types of memory
//The subscript should be the type field from GRUB's memory map
char const  * const mem_types[]={
	"Invalid",
	"Available",
	"Reserved",
	"ACPI Reclaimable",
	"ACPI NVS",
	"BADRAM"
};

Spinlock modifying_pmm;

static phys_mem_area_t *normal_mem_bitmap=nullptr;
size_t mem_bitmap_len=0;

//Setup by the linker to be at the start and end of the kernel.
extern const char kernel_start;
extern const char kernel_end;
const uintptr_t uint_kernel_start = reinterpret_cast<uintptr_t>(&kernel_start);
const uintptr_t uint_kernel_end = reinterpret_cast<uintptr_t>(&kernel_end);

//Start the physical memory manager
//mbp=MultiBoot Pointer (everything grub gives us)
//mbmp=MultiBoot Memory Pointer (grub lsmmap command output)
//len=number of memory areas
//Create a stack of pages for use
//Create+fill in the bitmap
//Call before paging is initialized, but after map_range is functional
int bootstrap_phys_mem_manager(multiboot_info_t *mbp){
	modifying_pmm.aquire_lock();
	multiboot_memory_map_t *mbmp=reinterpret_cast<multiboot_memory_map_t*>(mbp->mmap_addr);
	unsigned int mbmp_len=mbp->mmap_length;


	klog("Starting bootstrap_phys_mem_manager.");

	//Print debugging information
	klogf("Kernel starts at %p", static_cast<void const *>(&kernel_start));
	klogf("Kernel ends at %p", static_cast<void const *>(&kernel_end));
	klogf("It is %#lx bytes long.", uint_kernel_end-uint_kernel_start);

	//List all the memory
	for(unsigned int i=0;i<mbmp_len/sizeof(multiboot_memory_map_t);i++){
		mem_bitmap_len+=(mbmp+i)->len/PHYS_MEM_CHUNK_SIZE;
		klogf("Memory at %p for %llx with type %s.", reinterpret_cast<void const *>((mbmp+i)->addr), (mbmp+i)->len, mem_types[(mbmp+i)->type]);
	}

	klogf("Using %zd elements in array.", mem_bitmap_len);

	//Find where we can keep track of unused pages
	//Loop through the available memory again
	for(size_t i=0;i<mbmp_len/sizeof(multiboot_memory_map_t);i++){
		if((mbmp+i)->type==MULTIBOOT_MEMORY_AVAILABLE &&
				(mbmp+i)->len>(mem_bitmap_len*sizeof(phys_mem_area_t)))
		{
			//If the area starts before the kernel
			if(static_cast<uintptr_t>((mbmp+i)->addr)<=uint_kernel_start){
				//And has enough space
				if(((mbmp+i)->addr)+mem_bitmap_len < ((mbmp+i)->addr+(mbmp+i)->len)) {
						 //And has enough space before the kernel
						 if(((mbmp+i)->addr+(mem_bitmap_len*sizeof(phys_mem_area_t)))<uint_kernel_start){
							 klogf("Found memory at %p.", reinterpret_cast<void*>((mbmp+i)->addr));
							 normal_mem_bitmap=reinterpret_cast<phys_mem_area_t*>((mbmp+i)->addr);
						 }
						 //Or enough space after it
						 else if(((mbmp+i)->addr+(mbmp+i)->len)-uint_kernel_end>=(mem_bitmap_len*sizeof(phys_mem_area_t))){
							 klogf("Found memory at %p.", static_cast<void const *>(&kernel_end));
							 normal_mem_bitmap=reinterpret_cast<phys_mem_area_t *>(const_cast<char*>(&kernel_end));
						 }
						 else{
							 klog("Reached kernel.");
						 }
					 }
				else{
					klog("To short area.");
				}
			}
			//If it is after the kernel
			else if(static_cast<uintptr_t>((mbmp+i)->addr)>uint_kernel_end){
				//And has enough space
				if(((mbmp+i)->addr+
							(mem_bitmap_len*sizeof(phys_mem_area_t)))<
						((mbmp+i)->addr+(mbmp+i)->len))
				{
					klogf("Found memory at %p.", reinterpret_cast<void*>((mbmp+i)->addr));
					normal_mem_bitmap=reinterpret_cast<phys_mem_area_t*>((mbmp+i)->addr);
				}
				else{
					klog("To short area.");
				}
			}
			//It starts in the middle of the kernel?
			else{
				kwarnf("Memory starts at %p: in the middle of the kernel!", reinterpret_cast<void*>((mbmp+i)->addr));
			}
			//Note where we found the memory and stop searching
			klog("");
			klogf("Bitmap at %p.", static_cast<void*>(normal_mem_bitmap));
			klogf("Ends at %p.", static_cast<void*>(normal_mem_bitmap+mem_bitmap_len));
			klog("");
			break;
		}
	}

	//Loop through the memory again to fill in the bitmap
	for(unsigned int i=0;i<mbmp_len/sizeof(multiboot_memory_map_t);i++){
		bool available = ((mbmp+i)->type == MULTIBOOT_MEMORY_AVAILABLE);
		//TODO: delete low-level logging
		printf("i=%u, mbmp->len=%llu, mbmp->type=%d, available=%s\n", i, (mbmp+i)->len, (mbmp+i)->type, (available ? "true" : "false"));
		// fill in the array (pretend we loop over it)
		std::memset(&normal_mem_bitmap[(mbmp+i)->addr/PHYS_MEM_CHUNK_SIZE], static_cast<int>(available), bytes_to_pages(static_cast<uintptr_t>((mbmp+i)->len)));
	}
	puts("Bitmap filled...Marking kernel as used.");
	//Now mark the kernel as used
	std::memset(&normal_mem_bitmap[uint_kernel_start/PHYS_MEM_CHUNK_SIZE], INT_TRUE, bytes_to_pages(uint_kernel_end-uint_kernel_start));
	//And the bitmap itself
	std::memset(&normal_mem_bitmap[reinterpret_cast<uintptr_t>(normal_mem_bitmap)/PHYS_MEM_CHUNK_SIZE], INT_TRUE, bytes_to_pages(mem_bitmap_len));
	//And the first page (so it can be nullptr)
	//normal_mem_bitmap[0].in_use=true;
	//And map it
	printf("Bitmap points to %p in physical memory...", reinterpret_cast<void*>(normal_mem_bitmap));
	map_results mapping=map_range(normal_mem_bitmap, mem_bitmap_len*sizeof(phys_mem_area_t), reinterpret_cast<void**>(&normal_mem_bitmap), 0);
	if(mapping!=map_success){
		puts("");
		kcriticalf("Unable to map the physical memory manager (error code %d).", mapping);
		abort();
	}
	printf("and to %p in virtual memory.\n", reinterpret_cast<void*>(normal_mem_bitmap));
	klog("Ending bootstrap_phys_mem_manager.");
	modifying_pmm.release_lock();
	return 0;
}

//Find num unused (contiguous) pages
//modifying_pmm must be locked before calling this!
//returns nullptr if no RAM is available
static page find_free_pages(size_t num){
	//Abort if we don't have enough total RAM
	if(num>mem_bitmap_len){
		abort();
	}
	//Loop through the page table stopping when there can't be enough free space
	for(size_t where=0; where<mem_bitmap_len-num; where++){
		//If it is free
		if(!normal_mem_bitmap[where].in_use){
			//Start counting up to the number we need
			for(size_t count=0; count<num; count++){
				//If one isn't free
				if(normal_mem_bitmap[where+count].in_use){
					//Go back to searching for a free one after this
					where+=count;
					break;
				}
			}
			//If the free space wasn't long enough
			if(normal_mem_bitmap[where].in_use){
				//continue looking
				continue;
			}
			//There is enough free space!!
			//Loop through again
			else{
				return where*PHYS_MEM_CHUNK_SIZE;
			}
		}
	}
	//There isn't enough memory
	return nullptr;
}

//Reserve num pages starting at addr
//modifying_pmm must be locked before calling this!
static pmm_results internal_claim_mem_area(page const addr, size_t num, unsigned int opts [[maybe_unused]]){
	//Make sure we are given a valid pointer
	if(addr.isNull()){
		return pmm_null;
	}
	//Get the offset into the bitmap
	size_t offset=bytes_to_pages(addr);
	//Check if someone is trying to access beyond the end of RAM
	if(offset>mem_bitmap_len-num){
		return pmm_nomem;
	}
	//Double check that everything is free and claim it
	for(size_t count=0; count<num; count++){
		//If it is in use
		if(normal_mem_bitmap[offset+count].in_use){
			//Roll back our changes
			while(count>0){
				normal_mem_bitmap[offset+count-1].in_use=false;
				count--;
			}
			//And return an error
			return pmm_invalid;
		}
		//It is free
		else{
			//Claim it
			normal_mem_bitmap[offset+count].in_use=true;
		}
	}
	return pmm_success;
}

//Reserve len unused bytes from addr (if available)
pmm_results get_mem_area(void const * const addr, uintptr_t len, unsigned int opts){
	modifying_pmm.aquire_lock();
	pmm_results temp=internal_claim_mem_area(addr, bytes_to_pages(len), opts);
	modifying_pmm.release_lock();
	return temp;
}

//Aquire len unused bytes
pmm_results get_mem_area(void **addr, uintptr_t len, unsigned int opts){
	modifying_pmm.aquire_lock();
	*addr=find_free_pages(bytes_to_pages(len));
	if(*addr==nullptr){
		modifying_pmm.release_lock();
		return pmm_nomem;
	}
	pmm_results result=internal_claim_mem_area(*addr, bytes_to_pages(len), opts);
	modifying_pmm.release_lock();
	return result;
}

//Free len bytes from addr
pmm_results free_mem_area(void const * const addr, uintptr_t len, unsigned int opts [[maybe_unused]]){
	modifying_pmm.aquire_lock();
	//Figure out how many pages to mark free
	size_t num=bytes_to_pages(len);
	page base_page=addr;
	size_t base_index=base_page.getInt()/PHYS_MEM_CHUNK_SIZE;
	//Loop through all the pages
	for(size_t count=0; count<num; count++){
		//And make sure that none of them are already free
		if(!normal_mem_bitmap[base_index+count].in_use){
			//If any are, quit immediatly
			modifying_pmm.release_lock();
			return pmm_invalid;
		}
	}
	//Loop through again
	for(size_t count=0; count<num; count++){
		//And unmap everything
		//TODO: zero the pages?
		normal_mem_bitmap[base_index+count].in_use=false;
	}
	modifying_pmm.release_lock();
	return pmm_success;
}

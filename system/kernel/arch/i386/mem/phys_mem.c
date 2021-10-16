// SPDX-License-Identifier: MIT
// Copyright (c) 2021 James McNaughton Felder
#include "mem.h"
#include <drivers/serial.h>
#include <string.h>
#include <stdlib.h>

//General note:
//GRUB's pointers are stored as `unsigned long long` which (here at least) is the
//       same size as `unsigned long` which is the same size as `void*`.
//       To keep warnings down/allow -Werror to work I have to explicitly cast
//       from unsigned long long -> unsigned long -> void* whenever I use them
//       Please keep doing this unless there is a *clean* solution which works.
//       TODO: would it work to modify the multiboot.h file? (high chance of
//       backfiring on a transition to anything else)
//GRUB stores pointers as long long, so I have to cast to long before casting to void*
#define GRUBPTR2VOID (void*)(unsigned long)
#define GRUBPTR2CHAR (char*)(unsigned long)

//An array of strings for all types of memory
//The subscript should be the type field from GRUB's memory map
char *mem_types[]={
	"Invalid",
	"Available",
	"Reserved",
	"ACPI Reclaimable",
	"ACPI NVS",
	"BADRAM"
};

//A an array of markers for physical memory
//static phys_mem_area_t *mem_bitmap;
//size_t mem_bitmap_len=0;
//The first empty one
//size_t first_empty=0;

//static phys_mem_area_t mem_bitmap_bootstrap[0x1000000];
static phys_mem_area_t *mem_stack_bottom;
static phys_mem_area_t *mem_stack;

//Setup by the linker to be at the start and end of the kernel.
extern const char kernel_start;
extern const char kernel_end;
ptrdiff_t size;

//Fill the memory map
void populate_phys_mem_map(multiboot_info_t *mbp);

//Start the physical memory manager
//mbp=MultiBoot Pointer (everything grub gives us)
//mbmp=MultiBoot Memory Pointer (grub lsmmap command output)
//len=number of memory areas
//Create a stack of pages for use
int bootstrap_phys_mem_manager(multiboot_info_t *mbp){
	multiboot_memory_map_t *mbmp=(multiboot_memory_map_t*)(unsigned long)mbp->mmap_addr;
	unsigned int mbmp_len=mbp->mmap_length;

	size_t mem_stack_size=0;

	klog("Starting bootstrap_phys_mem_manager.");
	size=(ptrdiff_t)((char*)&kernel_end-(char*)&kernel_start)*sizeof(void*);

	//Print debugging information
	klogf("Kernel starts at %p", (void*)&kernel_start);
	klogf("Kernel ends at %p", (void*)&kernel_end);
	klogf("It is %#tx bytes long.", size);

	//Print the memory grub says is useable
	klogf("Useable memory:");
	for(unsigned int i=0;i<mbmp_len/sizeof(multiboot_memory_map_t);i++){
		if((mbmp+i)->type==MULTIBOOT_MEMORY_AVAILABLE){
			mem_stack_size+=(mbmp+i)->len/PHYS_MEM_CHUNK_SIZE;
			klogf("Memory at %p for %llx with type %s.", GRUBPTR2CHAR(mbmp+i)->addr, (mbmp+i)->len, mem_types[(mbmp+i)->type]);
		}
	}

	klogf("Need %zx bytes to store memory.", mem_stack_size*sizeof(phys_mem_area_t));

	//Find where we can keep track of unused pages
	//Loop through the available memory again
	for(size_t i=0;i<mbmp_len/sizeof(multiboot_memory_map_t);i++){
		if((mbmp+i)->type==MULTIBOOT_MEMORY_AVAILABLE &&
				(mbmp+i)->len>(mem_stack_size*sizeof(phys_mem_area_t)))
		{
			//If the area starts before the kernel
			if(GRUBPTR2CHAR(mbmp+i)->addr<=(char*)&kernel_start){
				//And has enough space
				if(GRUBPTR2CHAR((mbmp+i)->addr+
							//(mem_stack_size*sizeof(phys_mem_area_t)))<
							//GRUBPTR2CHAR((mbmp+i)->addr+(mbmp+i)->len))
					((phys_mem_area_t*)mem_stack_size<
					 (phys_mem_area_t*)(unsigned long)((mbmp+i)->addr+(mbmp+i)->len))))
					 {
						 //And has enough space before the kernel
						 if(GRUBPTR2CHAR((mbmp+i)->addr+(mem_stack_size*sizeof(phys_mem_area_t)))<(char*)&kernel_start){
							 klogf("Found memory at %p.", GRUBPTR2CHAR((mbmp+i)->addr));
							 mem_stack_bottom=(phys_mem_area_t*)(unsigned long)((mbmp+i)->addr);
							 mem_stack=(phys_mem_area_t*)(unsigned long)((mbmp+i)->addr)+mem_stack_size;
						 }
						 //Or enough space after it
						 else if(GRUBPTR2CHAR((mbmp+i)->addr+(mbmp+i)->len)-(char*)&kernel_end>=(ptrdiff_t)(mem_stack_size*sizeof(phys_mem_area_t))){
							 klogf("Found memory at %p.", &kernel_end);
							 mem_stack_bottom=(phys_mem_area_t*)&kernel_end;
							 mem_stack=(phys_mem_area_t*)&kernel_end+mem_stack_size;
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
			else if(GRUBPTR2CHAR(mbmp+i)->addr>(char*)&kernel_end){
				//And has enough space
				if(GRUBPTR2CHAR((mbmp+i)->addr+
							(mem_stack_size*sizeof(phys_mem_area_t)))<
						GRUBPTR2CHAR((mbmp+i)->addr+(mbmp+i)->len))
				{
					klogf("Found memory at %p.", GRUBPTR2CHAR((mbmp+i)->addr));
					mem_stack_bottom=(phys_mem_area_t*)(unsigned long)((mbmp+i)->addr);
					mem_stack=(phys_mem_area_t*)(unsigned long)((mbmp+i)->addr)+mem_stack_size;
				}
				else{
					klog("To short area.");
				}
			}
			//It starts in the middle of the kernel?
			else{
				kwarnf("Memory starts at %p: in the middle of the kernel!", GRUBPTR2CHAR(mbmp+i)->addr);
			}
			//Note where we found the memory and stop searching
			klog("");
			klogf("Stack at %p.", (void*)mem_stack_bottom);
			klogf("Reaches up to %p.", (void*)mem_stack);
			klogf("It has %zx entries.", mem_stack_size);
			klogf("Each item in the stack needs %zx bytes.", sizeof(phys_mem_area_t));
			klog("");
			break;
		}
	}

	//Keep track of where we are
	char *where;
	//And keep track of where we are keeping track of this
	phys_mem_area_t *ptr=mem_stack_bottom;
	//Discard used pages
	for(unsigned int i=0;i<mbmp_len/sizeof(multiboot_memory_map_t);i++){
		if((mbmp+i)->type==MULTIBOOT_MEMORY_AVAILABLE){
			for(where=GRUBPTR2CHAR(mbmp+i)->addr; where<GRUBPTR2CHAR((mbmp+i)->addr+(mbmp+i)->len); where+=PHYS_MEM_CHUNK_SIZE){
				//If the page has part of the kernel, discard it
				//If it starts before the kernel ends and ends after it starts
				if(where<&kernel_end && where+PHYS_MEM_CHUNK_SIZE>&kernel_start){
					ptr--;
					mem_stack--;
					continue;
				}
				//If we have a command line to preserve
				if(mbp->flags >> 2 & 0x1){
					//If it starts before the command line ends and ends after it starts
					if(
							where<GRUBPTR2CHAR mbp->cmdline &&
							where+PHYS_MEM_CHUNK_SIZE>(GRUBPTR2CHAR mbp->cmdline+strlen(GRUBPTR2CHAR mbp->cmdline))
					  ){
						//If we would be marking it as useable, don't
						ptr--;
						mem_stack--;
						continue;
					}
				}
				ptr->begin=where;
			}
			klogf("Memory at %p for %llx with type %s.", GRUBPTR2CHAR(mbmp+i)->addr, (mbmp+i)->len, mem_types[(mbmp+i)->type]);
		}
	}
	for(phys_mem_area_t *ptr=mem_stack_bottom; ptr<mem_stack; ptr++){
		//If the page has part of the kernel, discard it
		//If it starts before the kernel ends and ends after it starts
		if((char*)ptr->begin<&kernel_end && (char*)ptr->begin+PHYS_MEM_CHUNK_SIZE>&kernel_start){
			ptr--;
			mem_stack--;
			continue;
		}
		//If we have a command line to preserve
		if(mbp->flags >> 2 & 0x1){
			//If it starts before the command line ends and ends after it starts
			if(
					ptr->begin<GRUBPTR2VOID mbp->cmdline &&
					(char*)ptr->begin+PHYS_MEM_CHUNK_SIZE>(GRUBPTR2CHAR mbp->cmdline+strlen(GRUBPTR2CHAR mbp->cmdline))
			  ){
				ptr--;
				mem_stack--;
				continue;
			}
		}
	}
	klog("Ending bootstrap_phys_mem_manager.");
	return 0;
}

void *get_mem_area(){
	if(mem_stack<=mem_stack_bottom){
		kcritical("No more free memory.");
		abort();
	}
	void *retVal=mem_stack->begin;
	mem_stack--;
	return retVal;
}

int free_mem_area(void *mem_area){
	mem_stack++;
	mem_stack->begin=mem_area;
	return 0;
}

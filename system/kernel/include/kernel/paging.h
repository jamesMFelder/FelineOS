// SPDX-License-Identifier: MIT
// Copyright (c) 2021 James McNaughton Felder
#ifndef _KERN_PAGING_H
#define _KERN_PAGING_H 1

#include <cstdint>
#include <feline/fixed_width.h>
#include <kernel/mem.h>

#ifdef __cplusplus

//Map len bytes from phys_addr to virt_addr
map_results map_range(void const * const phys_addr, uintptr_t len, void const * const virt_addr, unsigned int opts);
//Map len bytes from phys_addr to any free virtual address
map_results map_range(void const * const phys_addr, uintptr_t len, void **virt_addr, unsigned int opts);
//Map len bytes from anywhere to virt_addr
map_results map_range(uintptr_t len, void const * const virt_addr, unsigned int opts);
//Map len bytes from anywhere to any free virtual address
map_results map_range(uintptr_t len, void **virt_addr, unsigned int opts);
//Unmap the page containing virt_addr
map_results unmap_page(page const virt_addr, unsigned int opts);
//Unmap all the pages from virt_addr to virt_addr+len
map_results unmap_range(void const * const virt_addr, uintptr_t len, unsigned int opts);

//tells unmap_{page,range} to deallocate the memory from the pmm
//TODO: get rid of this
#define PHYS_ADDR_AUTO 0b1u
//tell map_* to overwrite previous mappings
//TODO: restrict access to the core kernel
#define MAP_OVERWRITE 0b10u


//Call before setting up the PMM so it can map the pages it needs
int immediate_paging_initialization();
//Initialize paging
int setup_paging();

extern "C" void enable_paging(uintptr_t cr3);

//Invalidates the cpu's TLB for the page containing addr
void invlpg(page const addr);

#endif //__cplusplus

#endif //_KERN_PAGING_H

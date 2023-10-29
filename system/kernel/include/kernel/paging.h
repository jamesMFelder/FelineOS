/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2023 James McNaughton Felder */
#ifndef _KERN_PAGING_H
#define _KERN_PAGING_H 1

#include <cstdint>
#include <feline/fixed_width.h>
#include <kernel/mem.h>
#include <kernel/asm_compat.h>

//TODO: take size_t instead of uintptr_t for length
/* Map len bytes from phys_addr to virt_addr */
map_results map_range(void const * const phys_addr, uintptr_t len, void const * const virt_addr, unsigned int opts);
/* Map len bytes from phys_addr to any free virtual address */
map_results map_range(void const * const phys_addr, uintptr_t len, void **virt_addr, unsigned int opts);
/* Map len bytes from anywhere to virt_addr */
map_results map_range(uintptr_t len, void const * const virt_addr, unsigned int opts);
/* Map len bytes from anywhere to any free virtual address */
map_results map_range(uintptr_t len, void **virt_addr, unsigned int opts);
/* Unmap all the pages from virt_addr to virt_addr+len */
map_results unmap_range(void const * const virt_addr, uintptr_t len, unsigned int opts);

/* tells unmap_{page,range} to deallocate the memory from the pmm */
/* TODO: get rid of this */
#define PHYS_ADDR_AUTO 0b1u
/* tell map_* to overwrite previous mappings */
/* TODO: restrict access to the core kernel */
#define MAP_OVERWRITE 0b10u
/* tell map_* to map the memory as device memory instead of normal */
/* TODO: restrict access to the kernel */
#define MAP_DEVICE 0b100u

/* Call before setting up the PMM so it can map the pages it needs */
int immediate_paging_initialization();
/* Initialize paging */
int setup_paging();

#if defined(__i686__)
ASM void enable_paging(uintptr_t cr3);
#elif defined(__arm__)
ASM void enable_paging(uintptr_t ttbr0, uintptr_t ttbr1, uintptr_t ttbc);
#else
#error "Cannot detect architecture!"
#endif // __i686__ (else)

/* Invalidates the cpu's TLB for the page containing addr */
void invlpg(page const addr);

#endif /* _KERN_PAGING_H */

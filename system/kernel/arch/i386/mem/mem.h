// SPDX-License-Identifier: MIT
// Copyright (c) 2021 James McNaughton Felder
#ifndef _KERN_ARCH_MEM_H
#define _KERN_ARCH_MEM_H 1

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#include <kernel/multiboot.h>
#include <kernel/log.h>
#include <kernel/mem.h>

extern char *mem_types[];

#ifdef __cplusplus
extern "C"{
#endif
//Start the memory manager
int bootstrap_phys_mem_manager(multiboot_info_t *mbp);
#ifdef __cplusplus
}
#endif

//Used for a stack of pages
typedef struct phys_mem_area{
	//Is any part of this in use (including through a child)
	//bool in_use;

	//Where the memory starts
	void *begin;
} phys_mem_area_t;

#endif //_KERN_ARCH_MEM_H

/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2023 James McNaughton Felder */
#ifndef _KERN_ARCH_MEM_H
#define _KERN_ARCH_MEM_H 1

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#include <kernel/multiboot.h>
#include <kernel/mem.h>

/* Start the memory manager */
int bootstrap_phys_mem_manager(multiboot_info_t *mbp);

/* Used for a stack of pages */
typedef struct phys_mem_area{
	/* Is any part of this in use (including through a child) */
	bool in_use;
} phys_mem_area_t;

#endif /* _KERN_ARCH_MEM_H */

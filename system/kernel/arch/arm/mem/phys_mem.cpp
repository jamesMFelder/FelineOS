/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2023 James McNaughton Felder */
#include "mem.h"
#include <cassert>
#include <kernel/log.h>
#include <kernel/paging.h>
#include <kernel/vtopmem.h>
#include <kernel/mem.h>
#include <cstring>
#include <cstdlib>
#include <cinttypes>
#include <feline/fixed_width.h>
#include <feline/spinlock.h>
#include <feline/bool_int.h>

/* Setup by the linker to be at the start and end of the kernel. */
extern const char phys_kernel_start;
extern const char phys_kernel_end;
extern const char kernel_start;
extern const char kernel_end;

/* Start the physical memory manager */
/* mbp=MultiBoot Pointer (everything grub gives us) */
/* mbmp=MultiBoot Memory Pointer (grub lsmmap command output) */
/* len=number of memory areas */
/* Create a stack of pages for use */
/* Create+fill in the bitmap */
/* Call after paging is active */
int bootstrap_phys_mem_manager(multiboot_info_t*){
	return 0;
}

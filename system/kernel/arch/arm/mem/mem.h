/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2023 James McNaughton Felder */
#ifndef _KERN_ARCH_MEM_H
#define _KERN_ARCH_MEM_H 1

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#include <kernel/mem.h>
#include <kernel/devicetree.h>

/* Start the memory manager */
int bootstrap_phys_mem_manager(fdt_header *devicetree);

#endif /* _KERN_ARCH_MEM_H */

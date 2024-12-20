/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2023 James McNaughton Felder */
#ifndef _KERN_ARCH_MEM_H
#define _KERN_ARCH_MEM_H 1

#include <kernel/devicetree.h>
#include <kernel/mem.h>
#include <kernel/phys_addr.h>

/* Start the memory manager */
int bootstrap_phys_mem_manager(PhysAddr<fdt_header const> devicetree);

#endif /* _KERN_ARCH_MEM_H */

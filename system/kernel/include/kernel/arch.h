/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2023 James McNaughton Felder */
#ifndef _KERN_BOOT_H
#define _KERN_BOOT_H 1

#include <kernel/multiboot.h>
#include <kernel/devicetree.h>
#include <kernel/asm_compat.h>

/* Call before kernel_main(); */
/* What you can use: */
/*	the stack (not a lot of it) */
/*	32-bit registers (following the calling convention) */
/*	serial_{putc,writestr} */
/* Missing features include: */
/*	paging */
/*	heap */
/*	screen output */
/* See the definition for dependencies */
#if defined(__i686__)
ASM int early_boot_setup(multiboot_info_t *mbp);
#elif defined(__arm__)
ASM int early_boot_setup(fdt_header *fdt);
#else
#error "No architecture defined! Cannot declare early_boot_setup with the appropriate argument!"
#endif
int boot_setup();

#endif /* _KERN_BOOT_H */

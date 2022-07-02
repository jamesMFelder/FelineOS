/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2021 James McNaughton Felder */
#ifndef _KERN_BOOT_H
#define _KERN_BOOT_H 1

#include <kernel/multiboot.h>
#include <kernel/asm_compat.h>

/* Call before kernel_main(); */
/* What you can use: */
/* 	the stack (not a lot of it) */
/* 	32-bit registers (following the calling convention) */
/* 	serial_{putc,writestr} */
/* Missing features include: */
/* 	paging */
/* 	heap */
/* 	screen output */
/* See the definition for dependencies */
ASM int early_boot_setup(multiboot_info_t *mbp);
int boot_setup();

#endif /* _KERN_BOOT_H */

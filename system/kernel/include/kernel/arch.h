// SPDX-License-Identifier: MIT
// Copyright (c) 2021 James McNaughton Felder
#ifndef _KERN_BOOT_H
#define _KERN_BOOT_H 1

#include <kernel/multiboot.h>

int boot_setup(multiboot_info_t *mbp);

#endif //_KERN_BOOT_H

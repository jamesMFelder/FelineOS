// SPDX-License-Identifier: MIT
// Copyright (c) 2021 James McNaughton Felder
#ifndef _KERN_BOOT_H
#define _KERN_BOOT_H

#include <kernel/multiboot.h>

#ifdef __cplusplus
extern "C"{
#endif
int boot_setup(multiboot_info_t *mbp);
#ifdef __cplusplus
}
#endif

#endif //_KERN_BOOT_H

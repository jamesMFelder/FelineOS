#ifndef _KERN_BOOT_H
#define _KERN_BOOT_H

#include <kernel/multiboot.h>

int boot_setup(multiboot_info_t *mbp);

#endif //_KERN_BOOT_H

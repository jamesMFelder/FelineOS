/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2025 James McNaughton Felder */
#ifndef _KERN_MOD_H
#define _KERN_MOD_H 1

#include <cstddef>
#include <feline/kallocator.h>
#include <feline/kstring.h>
#include <feline/kvector.h>
#include <feline/logger.h>
#include <kernel/mem.h>
#include <kernel/multiboot.h>
#include <kernel/paging.h>
#include <kernel/phys_addr.h>
#include <kernel/vtopmem.h>

struct Module {
		KConstString cmdline;
		KVector<std::byte, KGeneralAllocator<std::byte>> data;

		Module(multiboot_module_t grub_mod) {
			char const *mapped_cmdline;
			map_results cmdline_mapping =
				map_range(PhysAddr<const void>(grub_mod.cmdline), 4_KiB,
			              reinterpret_cast<void const **>(&mapped_cmdline), 0);
			if (cmdline_mapping != map_success) {
				kError() << "Unable to map cmdline. Error "
						 << dec(cmdline_mapping);
				cmdline = KConstString();
			} else {
				cmdline = KConstString(mapped_cmdline,
				                       strnlen(mapped_cmdline, 4_KiB));
			}

			std::byte *mod;
			map_results mod_mapping =
				map_range(PhysAddr<std::byte>(grub_mod.mod_start),
			              grub_mod.mod_end - grub_mod.mod_start,
			              reinterpret_cast<void **>(&mod), 0);
			if (mod_mapping != map_success) {
				kError() << "Unable to map module. Error "
						 << dec(cmdline_mapping);
			} else {
				data = KVector<std::byte, KGeneralAllocator<std::byte>>(
					mod, grub_mod.mod_end - grub_mod.mod_start);
				/* Use PHYS_ADDR_AUTO, because this memory was marked as
				 * reserved by Grub but since we have copied the module out of
				 * it (TODO: don't do that, it's slow) we can use the memory it
				 * used to be in*/
				unmap_range(mod, grub_mod.mod_end - grub_mod.mod_start,
				            PHYS_ADDR_AUTO);
			}
		}
};

#endif /* _KERN_MOD_H */

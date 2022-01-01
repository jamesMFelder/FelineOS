// SPDX-License-Identifier: MIT
// Copyright (c) 2021 James McNaughton Felder
#include <kernel/arch.h>

#include "gdt/gdt.h"
#include <kernel/interrupts.h>
#include <kernel/paging.h>
#include "mem/mem.h"
#include <cstring>

static char grub_cmdline[4096]="";

static void save_grub_params(multiboot_info_t *mbp){
	if(mbp->flags >> 2 & 0x1){
		size_t len=strlcpy(grub_cmdline, reinterpret_cast<char*>(mbp->cmdline), 4096);
		if(len>4096){
			kerror("We were given too long a command line, truncating to 4096 characters before continuing.");
			kerror("Here is the full command line:");
			kerrorf("%s", reinterpret_cast<char*>(mbp->cmdline));
		}
	}
	return;
}

int boot_setup(multiboot_info_t *mbp){
	save_grub_params(mbp);

	klogf("Command line: %s", grub_cmdline);

	//Setup the Global Descriptor Table to do nothing
	disable_gdt();

	//Enable interrupts
	idt_init();

	//Setup paging
	immediate_paging_initialization();

	bootstrap_phys_mem_manager(mbp);

	//Turn on paging
	setup_paging();

	return 0;
}

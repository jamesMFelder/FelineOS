// SPDX-License-Identifier: MIT
// Copyright (c) 2021 James McNaughton Felder
#include <kernel/arch.h>

#include "gdt/gdt.h"
#include <kernel/idt.h>
#include "mem/mem.h"
#include <kernel/tty.h>

int boot_setup(multiboot_info_t *mbp){
	//Set up the terminal for logging
	terminal_initialize();

	if(mbp->flags >> 2 & 0x1){
		klogf("Command line: %s.", (char*)(unsigned long)mbp->cmdline);
	}

	//Setup the Global Descriptor Table to do nothing
	disable_gdt();

	//Enable interrupts
	idt_init();

	bootstrap_phys_mem_manager(mbp);

	return 0;
}

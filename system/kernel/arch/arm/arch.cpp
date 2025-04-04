/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2023 James McNaughton Felder */

#include "mem/mem.h"
#include <cinttypes>
#include <cstring>
#include <drivers/framebuffer.h>
#include <drivers/serial.h>
#include <feline/logger.h>
#include <feline/settings.h>
#include <kernel/arch.h>
#include <kernel/devicetree.h>
#include <kernel/halt.h>
#include <kernel/interrupts.h>
#include <kernel/misc.h>
#include <kernel/paging.h>
#include <kernel/phys_addr.h>
#include <kernel/vtopmem.h>

framebuffer fb;

static void screen_init() {}

/* Dependencies:
   Setup the IDT before any errors can occur
   Take control of paging (start the VMM) before the PMM or devicetree need it
   Turn on the serial port as soon as we can map it
   Setup the device tree before the PMM
   Initialize the framebuffer
Flexibility:
In an ideal world we should be able to setup the IDT anytime before paging.
If you write a real serial port driver using interrupts, note that you won't
be able to log anything until the IDT is setup (attempts will triple-fault)
Also note that the interrupts actually log stuff, so watch out!
After this we should be good to go! */
int early_boot_setup(uintptr_t devicetree_header_addr) {
	PhysAddr<fdt_header const> devicetree(devicetree_header_addr);
	idt_init(); /* Actually display an error if we have a problem: don't just
	               triple fault */
	if (setup_paging() != 0) {
		halt();
	} /* Take control of it from the assembly! */
	init_serial(); /* We can't do any logging before this gets setup */
	Settings::Logging::critical.initialize(write_serial);
	Settings::Logging::error.initialize(write_serial);
	Settings::Logging::warning.initialize(write_serial);
	Settings::Logging::log.initialize(write_serial);
	Settings::Logging::debug.initialize(write_serial);
	bootstrap_phys_mem_manager(
		devicetree); /* Get the physical memory manager working */
	screen_init();   /* Initialize the framebuffer */
	return 0;
}

int boot_setup() {
	/* Re-initialized logging */
	Settings::Logging::critical.initialize(write_serial);
	Settings::Logging::error.initialize(write_serial);
	Settings::Logging::warning.initialize(write_serial);
	Settings::Logging::log.initialize(write_serial);
	Settings::Logging::debug.initialize(write_serial);
	for_each_prop_in_node(
		"chosen",
		[](fdt_struct_entry *entry, devicetree_cell_size, char *, void *) {
			if (strcmp(entry->node_name, "cmdline")) {
				Settings::Misc::commandline.initialize(
					KStringView(reinterpret_cast<char *>(&(entry->prop.value)),
			                    entry->prop.len));
			}
		},
		nullptr);
	return 0;
}

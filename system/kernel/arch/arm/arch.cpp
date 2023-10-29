/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2023 James McNaughton Felder */

#include <kernel/arch.h>
#include <kernel/interrupts.h>
#include <kernel/paging.h>
#include "mem/mem.h"
#include <cstring>
#include <kernel/misc.h>
#include <drivers/framebuffer.h>
#include <drivers/serial.h>
#include <cstdlib>
#include <cinttypes>
#include <kernel/settings.h>
#include <kernel/vtopmem.h>

framebuffer fb;

static void screen_init(){
}


/* Dependencies:
   Turn on the serial port before we log anything
   Setup the IDT before any errors can occur
   Take control of paging (start the VMM) before the PMM or devicetree need it
   Setup the device tree before the PMM
   Initialize the framebuffer
Flexibility:
In an ideal world we should be able to setup the IDT anytime before paging.
If you write a real serial port driver using interrupts, note that you won't
be able to log anything until the IDT is setup (attempts will triple-fault)
Also note that the interrupts actually log stuff, so watch out!
After this we should be good to go! */
int early_boot_setup(fdt_header *devicetree){
	init_serial(); /* We can't do any logging before this gets setup */
	Settings::Logging::critical.initialize(write_serial);
	Settings::Logging::error.initialize(write_serial);
	Settings::Logging::warning.initialize(write_serial);
	Settings::Logging::log.initialize(write_serial);
	Settings::Logging::debug.initialize(write_serial);
	idt_init(); /* Actually display an error if we have a problem: don't just triple fault */
	setup_paging(); /* Take control of it from the assembly! */
	klogf("Device tree should be at %p", static_cast<void*>(devicetree));
	klogf("Devicetree:\n\tmagic: %#" PRIx32 "\n\toff_mem_rsvmap: %#" PRIx32,
			static_cast<uint32_t>(read_pmem(devicetree).magic),
			static_cast<uint32_t>(read_pmem(devicetree).off_mem_rsvmap)
		 );
	//Print all the structures
	devicetree = init_devicetree(devicetree);
	bootstrap_phys_mem_manager(devicetree); /* Get the physical memory manager working */
	screen_init(); /* Initialize the framebuffer */
	return 0;
}

int boot_setup(){
	for_each_prop_in_node("chosen", []
			(fdt_struct_entry *entry, devicetree_cell_size , char *, void *_[[maybe_unused]])
			{
				if (strcmp(entry->node_name, "cmdline")) {
					Settings::Misc::commandline.initialize(std::string_view(reinterpret_cast<char*>(&(entry->prop.value)), entry->prop.len));
				}
			}, nullptr);
	return 0;
}

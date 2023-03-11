/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2023 James McNaughton Felder */
#include <feline/reverse_endian.h>
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
#include <kernel/vtopmem.h>

framebuffer fb;

static void screen_init(){
}

char grub_cmdline[4096]="";

/* Dependencies:
   Turn on the serial port before we log anything
   Setup the PMM before the IDT clobbers the devicetree in QEMU (don't call it until paging is ON)
   Setup the IDT before any errors can occur
   Initialize the framebuffer
Flexibility:
The devicetree should be relocatable, the problem is where can it go.
In an ideal world we should be able to setup the IDT anytime before paging.
If you write a real serial port driver using interrupts, note that you won't
be able to log anything until the IDT is setup (attempts will triple-fault)
Also note that the interrupts actually log stuff, so watch out!
After this we should be good to go! */
int early_boot_setup(fdt_header *devicetree){
	init_serial(); /* We can't do any logging before this gets setup */
	klogf("Device tree should be at %p", static_cast<void*>(devicetree));
	klogf("Devicetree:\n\tmagic: %#" PRIx32 "\n\toff_mem_rsvmap: %#" PRIx32,
			static_cast<uint32_t>(devicetree->magic),
			static_cast<uint32_t>(devicetree->off_mem_rsvmap)
		 );
	//Print all the structures
	init_devicetree(devicetree);
	bootstrap_phys_mem_manager(devicetree); /* Get the physical memory manager working */
	idt_init(); /* Actually display an error if we have a problem: don't just triple fault */
	//setup_paging(); /* Take control of it from the assembly! */
	screen_init(); /* Initialize the framebuffer */
	return 0;
}

int boot_setup(){
	klogf("Command line: %s", grub_cmdline);
	return 0;
}

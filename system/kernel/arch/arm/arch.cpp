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
#include <kernel/vtopmem.h>

framebuffer fb;

static void screen_init(){
}

char grub_cmdline[4096]="";

/* Dependencies:
	Setup the GDT ASAP
	Turn on the serial port before we log anything
	Setup the IDT before any errors can occur
	Initial paging setup so map_range doesn't fail (not turning it on)
	Setup the PMM (don't call it until paging is ON)
	Turn on paging
	Initialize the framebuffer
Flexibility:
	Saving grub info (esp. the command line) and ACPI tables after the PMM would
		be ideal in that we could save a variable length command line, but messier
		and more likely to end up overwriting them (esp. in low-mem computers).
		And honestly, why should we support more than a kilobyte long command line?
	In an ideal world we should be able to setup the IDT anytime before paging.
	If you write a real serial port driver using interrupts, note that you won't
		be able to log anything until the IDT is setup (attempts will triple-fault)
		Also note that the interrupts actually log stuff, so watch out!
After this we should be good to go! */
int early_boot_setup(multiboot_info_t *mbp [[maybe_unused]]){
	init_serial(); /* We can't do any logging before this gets setup */
	idt_init(); /* Actually display an error if we have a problem: don't just triple fault */
	//setup_paging(); /* Take control of it from the assembly! */
	//bootstrap_phys_mem_manager(mbp); /* Get the physical memory manager working */
	screen_init(); /* Initialize the framebuffer */
	return 0;
}

int boot_setup(){
	klogf("Command line: %s", grub_cmdline);
	return 0;
}

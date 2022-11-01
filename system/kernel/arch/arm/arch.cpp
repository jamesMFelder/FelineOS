/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2021 James McNaughton Felder */
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
	/*klogf("Framebuffer at %p.", reinterpret_cast<void*>(mbp.framebuffer_addr));
	klogf("Frame buffer pitch (in bytes): %u.", mbp.framebuffer_pitch);
	klogf("It is %ux%u (in pixels).", mbp.framebuffer_width, mbp.framebuffer_height);
	klogf("With %" PRIu8 " bits per pixel.", mbp.framebuffer_bpp);
	switch(mbp.framebuffer_type){
		case MULTIBOOT_FRAMEBUFFER_TYPE_INDEXED:
			klogf("Framebuffer type: %s.", "indexed");
			klogf("Palette at %p with %d colors.", reinterpret_cast<void*>(mbp.fb_palette.framebuffer_palette_addr), mbp.fb_palette.framebuffer_palette_num_colors);
			break;
		case MULTIBOOT_FRAMEBUFFER_TYPE_RGB:
			klogf("Framebuffer type: %s.", "RGB");
			klogf("Red field position: %" PRIu8 ", mask size: %" PRIu8, mbp.fb_rgb.framebuffer_red_field_position, mbp.fb_rgb.framebuffer_red_mask_size);
			klogf("Green field position: %" PRIu8 ", mask size: %" PRIu8, mbp.fb_rgb.framebuffer_green_field_position, mbp.fb_rgb.framebuffer_green_mask_size);
			klogf("Blue field position: %" PRIu8 ", mask size: %" PRIu8, mbp.fb_rgb.framebuffer_blue_field_position, mbp.fb_rgb.framebuffer_blue_mask_size);
			break;
		case MULTIBOOT_FRAMEBUFFER_TYPE_EGA_TEXT:
			klogf("Framebuffer type: %s.", "EGA text");
			break;
	}
	int fb_init_rval=fb.init(reinterpret_cast<pixel_bgr_t*>(mbp.framebuffer_addr), static_cast<uint16_t>(mbp.framebuffer_width), static_cast<uint16_t>(mbp.framebuffer_height), static_cast<uint16_t>(mbp.framebuffer_pitch), mbp.framebuffer_bpp);
	if(fb_init_rval!=0){
		kerrorf("Unable to initialize framebuffer! Failed with error %d.", fb_init_rval);
		abort();
	}*/
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
int early_boot_setup(multiboot_info_t *mbp){
	init_serial(); /* We can't do any logging before this gets setup */
	writestr_serial("Hello world!\r\n");
	idt_init(); /* Actually display an error if we have a problem: don't just triple fault */
	setup_paging(); /* Take control of it from the assembly! */
	bootstrap_phys_mem_manager(mbp); /* Get the physical memory manager working */
	screen_init(); /* Initialize the framebuffer */
	return 0;
}

int boot_setup(){
	klogf("Command line: %s", grub_cmdline);
	return 0;
}

/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2023 James McNaughton Felder */
#include <kernel/arch.h>

#include "gdt/gdt.h"
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

static void screen_init(multiboot_info_t mbp){
	klogf("Framebuffer at %p.", reinterpret_cast<void*>(mbp.framebuffer_addr));
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
	}
}

char grub_cmdline[4096]="";
multiboot_uint32_t grub_flags;

static void save_grub_params(multiboot_info_t * const phys_mbp){
	multiboot_info_t mbp=read_pmem(phys_mbp);
	grub_flags=mbp.flags;
	if(get_flag(grub_flags, MULTIBOOT_INFO_CMDLINE)){
		char *mapped_cmdline;
		map_results cmdline_mapping=map_range(reinterpret_cast<char*>(mbp.cmdline), 4_KiB, reinterpret_cast<void**>(&mapped_cmdline), 0);
		if (cmdline_mapping != map_success) {
			kerrorf("Unable to map information from Grub. Error %d.", cmdline_mapping);
			goto after_cmdline;
		}
		size_t offset=0;
		for (offset=0; offset < 4096; ++offset) {
			grub_cmdline[offset]=mapped_cmdline[offset];
			if (grub_cmdline[offset] == '\0') {
				break;
			}
		}
		//size_t len=strlcpy(grub_cmdline, reinterpret_cast<char*>(mbp->cmdline), 4096);
		if(offset==4096){
			kerror("We were given too long a command line, truncating to 4096 characters.");
		}
		unmap_range(mapped_cmdline, 4096, 0);
	}
after_cmdline: //Jump here if you need to abort processing the command line.
	if(get_flag(grub_flags, MULTIBOOT_INFO_FRAMEBUFFER_INFO)){
		screen_init(mbp);
	}
	else{
		/* TODO: actually do this */
		kwarn("Not finding screen info from GRUB, using serial port only.");
	}
	return;
}

/* Dependencies:
	Setup the GDT ASAP
	Turn on the serial port before we log anything
	Setup the IDT before any errors can occur
	Initial paging setup so map_range doesn't fail (not turning it on)
	Save grub info before it gets clobbered
	TODO: save ACPI tables before they get clobbered
	Setup the PMM (don't call it until paging is ON)
	Turn on paging
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
	disable_gdt(); /* We don't use it because it's not cross platform */
	init_serial(); /* We can't do any logging before this gets setup */
	idt_init(); /* Actually display an error if we have a problem: don't just triple fault */
	setup_paging(); /* Take control of it from the assembly! */
	save_grub_params(mbp);
	bootstrap_phys_mem_manager(mbp); /* Get the physical memory manager working */
	return 0;
}

int boot_setup(){
	klogf("Command line: %s", grub_cmdline);
	return 0;
}

// SPDX-License-Identifier: MIT
// Copyright (c) 2021 James McNaughton Felder
#include <cstdio>

#include <cstdlib>

#include <cinttypes>

#include <kernel/arch.h>

#include <kernel/log.h>

#include <kernel/cpuid.h>

#include <kernel/multiboot.h>

#include <kernel/misc.h>

#include <drivers/serial.h>

#include <kernel/mem.h>

//Setup by the linker to be at the start and end of the kernel.
extern const char kernel_start;
extern const char kernel_end;

extern "C"{
void kernel_main(multiboot_info_t *mbp, unsigned int magic) {
	char vendor[13];
	init_serial();
	boot_setup(mbp);

	//TODO: can we avoid relying on multiboot
	if(magic!=MULTIBOOT_BOOTLOADER_MAGIC){
		kerrorf("Magic is %X, should be %X.", magic, MULTIBOOT_BOOTLOADER_MAGIC);
		kerror("Not booted by a multiboot bootloader.");
		abort();
	}

	//Announce that we are loaded
	klog("Hello kernel world!");

	if(cpuid_supported()){
		cpuid_vendor(vendor);
		printf("Your cpu is %s.\n", vendor);
	}

	printf("Kernel starts at %p\n", &kernel_start);
	printf("Kernel ends at %p\n", &kernel_end);

	if(get_flag(mbp->flags, MULTIBOOT_INFO_CMDLINE)){
		klogf("Command line=%s", (char*)(unsigned long)mbp->cmdline);
	}

	printf("Testing memory allocation.\n");
	void *old_mem_ptr=get_mem_area();
	printf("Got a page at %p.\n", old_mem_ptr);
	printf("Freeing an allocated area returns %d.\n", free_mem_area(old_mem_ptr));
	void *mem_ptr=get_mem_area();
	printf("Got another page at %p.\n", mem_ptr);
	if(mem_ptr==old_mem_ptr){
		//terminal_setcolor(color_ok_dark);
		printf("Addresses match!\n");
		//terminal_setcolor(color_normal_light);
	}
	else{
		//terminal_setcolor(color_bad_dark);
		printf("Addresses do not match!\n");
		//terminal_setcolor(color_normal_light);
	}

	if(!get_flag(mbp->flags, MULTIBOOT_INFO_MEM_MAP)){
		kcritical("No valid memory map.");
		kcritical("In grub, type 'c' for a command line and then type 'displaysmem' or 'lsmmap' to see the memory it thinks exists.");
		abort();
	}

	if(get_flag(mbp->flags, MULTIBOOT_INFO_FRAMEBUFFER_INFO)){
		klogf("Framebuffer at %p.", (void*)mbp->framebuffer_addr);
		klogf("Frame buffer pitch (in bytes): %u.", mbp->framebuffer_pitch);
		klogf("Framebuffer type: %u.", mbp->framebuffer_type);
		klogf("It is %ux%u (in pixels).", mbp->framebuffer_width, mbp->framebuffer_height);
		switch(mbp->framebuffer_type){
			case MULTIBOOT_FRAMEBUFFER_TYPE_INDEXED:
				klogf("Framebuffer type: %s.", "indexed");
				klogf("Palette at %p with %d colors.", (void*)mbp->fb_palette.framebuffer_palette_addr, mbp->fb_palette.framebuffer_palette_num_colors);
				break;
			case MULTIBOOT_FRAMEBUFFER_TYPE_RGB:
				klogf("Framebuffer type: %s.", "RGB");
				klogf("Red field position: %" PRIu8 ", mask size: %" PRIu8, mbp->fb_rgb.framebuffer_red_field_position, mbp->fb_rgb.framebuffer_red_mask_size);
				klogf("Green field position: %" PRIu8 ", mask size: %" PRIu8, mbp->fb_rgb.framebuffer_green_field_position, mbp->fb_rgb.framebuffer_green_mask_size);
				klogf("Blue field position: %" PRIu8 ", mask size: %" PRIu8, mbp->fb_rgb.framebuffer_blue_field_position, mbp->fb_rgb.framebuffer_blue_mask_size);
				break;
			case MULTIBOOT_FRAMEBUFFER_TYPE_EGA_TEXT:
				klogf("Framebuffer type: %s.", "EGA text");
				break;
		}
	}
	else{
		//TODO: actually do this
		kwarn("Not finding a framebuffer from GRUB, falling back to text mode.");
	}
	kcritical("Nothing to do... Pausing now."); //boot.S should hang if we return
}
}

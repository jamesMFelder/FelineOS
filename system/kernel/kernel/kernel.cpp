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

#include <drivers/framebuffer.h>

#include <kernel/mem.h>

#include <sys/syscall.h>

#include <kernel/backtrace.h>

#include <feline/str.h>

//Setup by the linker to be at the start and end of the kernel.
extern const char kernel_start;
extern const char kernel_end;

extern "C"{
void kernel_main(multiboot_info_t *mbp, unsigned int magic) {
	init_serial();

	//TODO: can we avoid relying on multiboot
	if(magic!=MULTIBOOT_BOOTLOADER_MAGIC){
		kerrorf("Magic is %X, should be %X.", magic, MULTIBOOT_BOOTLOADER_MAGIC);
		kerror("Not booted by a multiboot bootloader.");
		abort();
	}

	if(!(mbp->flags >> 6 & 0x1)){
		kcritical("No valid memory map.");
		kcritical("In grub, type 'c' for a command line and then type 'displaysmem' or 'lsmmap' to see the memory it thinks exists.");
		abort();
	}

	boot_setup(mbp);

	//Announce that we are loaded
	klog("Hello kernel world!");

	if(cpuid_supported()){
		unsigned char vendor[13];
		cpuid_vendor(vendor);
		printf("Your cpu is %s.\n", vendor);
	}

	printf("Kernel starts at %p\n", static_cast<const void *>(&kernel_start));
	printf("Kernel ends at %p\n", static_cast<const void *>(&kernel_end));

	if(get_flag(mbp->flags, MULTIBOOT_INFO_CMDLINE)){
		klogf("Command line=%s", reinterpret_cast<char*>(mbp->cmdline));
	}

	printf("Testing memory allocation.\n");
	void *old_mem_ptr=get_mem_area();
	printf("Got a page at %p.\n", old_mem_ptr);
	printf("Freeing an allocated area returns %d.\n", free_mem_area(old_mem_ptr));
	void *mem_ptr=get_mem_area();
	printf("Got another page at %p.\n", mem_ptr);
	if(mem_ptr==old_mem_ptr){
		printf("Addresses match!\n");
	}
	else{
		printf("Addresses do not match!\n");
	}

	if(!get_flag(mbp->flags, MULTIBOOT_INFO_MEM_MAP)){
		kcritical("No valid memory map.");
		kcritical("In grub, type 'c' for a command line and then type 'displaysmem' or 'lsmmap' to see the memory it thinks exists.");
		abort();
	}

	if(get_flag(mbp->flags, MULTIBOOT_INFO_FRAMEBUFFER_INFO)){
		klogf("Framebuffer at %p.", reinterpret_cast<void*>(mbp->framebuffer_addr));
		klogf("Frame buffer pitch (in bytes): %u.", mbp->framebuffer_pitch);
		klogf("It is %ux%u (in pixels).", mbp->framebuffer_width, mbp->framebuffer_height);
		klogf("With %" PRIu8 " bits per pixel.", mbp->framebuffer_bpp);
		switch(mbp->framebuffer_type){
			case MULTIBOOT_FRAMEBUFFER_TYPE_INDEXED:
				klogf("Framebuffer type: %s.", "indexed");
				klogf("Palette at %p with %d colors.", reinterpret_cast<void*>(mbp->fb_palette.framebuffer_palette_addr), mbp->fb_palette.framebuffer_palette_num_colors);
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
		framebuffer fb; //setup the framebuffer
		fb.init(reinterpret_cast<pixel_bgr_t*>(mbp->framebuffer_addr), mbp->framebuffer_width, mbp->framebuffer_height, mbp->framebuffer_pitch, mbp->framebuffer_bpp);
		//Fill each corner with a color
		pixel_t p={255, 255, 255}; //white
		uint16_t maxX, maxY;
		fb.getMax(&maxX, &maxY); //get the maximum sizes
		fb.putRect(0,      0,      maxX/2,   maxY/2, p); //upper left
		p={255, 0, 0}; //red
		fb.putRect(maxX/2, 0,      maxX/2-1, maxY/2, p); //uppper right
		p={0, 255, 0}; //green
		fb.putRect(0,      maxY/2, maxX/2,   maxY/2-1, p); //lower left
		p={0, 0, 255}; //blue
		fb.putRect(maxX/2, maxY/2, maxX/2-1, maxY/2-1, p); //lower right
	}
	else{
		//TODO: actually do this
		kwarn("Not finding screen info GRUB, using serial port only.");
	}
	printf("Testing an syscall...\n");
	printf("It returned %ld.\n", syscall(0));
	kcritical("Nothing to do... Aborting now."); //boot.S should hang if we return
	abort(); //But we're just going to crash (and print a backtrace) for now.
}
}

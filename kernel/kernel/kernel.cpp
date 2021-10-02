// SPDX-License-Identifier: MIT
// Copyright (c) 2021 James McNaughton Felder
#include <cstdio>

#include <cstdlib>

#include <kernel/arch.h>

#include <kernel/tty.h>

#include <kernel/log.h>

#include <kernel/cpuid.h>

#include <kernel/multiboot.h>

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
	terminal_setcolor(color_ok_dark);
	klog("Hello kernel world!");
	terminal_setcolor(color_normal_light);

	if(cpuid_supported()){
		cpuid_vendor(vendor);
		printf("Your cpu is %s.\n", vendor);
	}

	printf("Kernel starts at %p\n", &kernel_start);
	printf("Kernel ends at %p\n", &kernel_end);

	if(mbp->flags >> 0x2){
		klogf("Command line=%s", (char*)(unsigned long)mbp->cmdline);
	}

	printf("Testing memory allocation.\n");
	void *old_mem_ptr=get_mem_area();
	printf("Got a page at %p.\n", old_mem_ptr);
	printf("Freeing an allocated area returns %d.\n", free_mem_area(old_mem_ptr));
	void *mem_ptr=get_mem_area();
	printf("Got another page at %p.\n", mem_ptr);
	if(mem_ptr==old_mem_ptr){
		terminal_setcolor(color_ok_dark);
		printf("Addresses match!\n");
		terminal_setcolor(color_normal_light);
	}
	else{
		terminal_setcolor(color_bad_dark);
		printf("Addresses do not match!\n");
		terminal_setcolor(color_normal_light);
	}

	if(!(mbp->flags >> 6 & 0x1)){
		kcritical("No valid memory map.");
		kcritical("In grub, type 'c' for a command line and then type 'displaysmem' or 'lsmmap' to see the memory it thinks exists.");
		abort();
	}

	terminal_setcolor(color_bad_dark);
	kcritical("Nothing to do... Pausing now."); //boot.S should hang if we return
	terminal_setcolor(color_normal_light);
}
}

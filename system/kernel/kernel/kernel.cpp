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
#include <sys/syscall.h>
#include <kernel/backtrace.h>
#include <feline/str.h>
#include <kernel/mem.h>
#include <kernel/asm_compat.h>

//Setup by the linker to be at the start and end of the kernel.
extern const char kernel_start;
extern const char kernel_end;

ASM void kernel_main(multiboot_info_t *mbp, unsigned int magic);

void kernel_main(multiboot_info_t *mbp [[maybe_unused]], unsigned int magic [[maybe_unused]]){
	boot_setup();

	//Announce that we are loaded
	klog("Hello kernel world!");

	if(cpuid_supported()){
		unsigned char vendor[13];
		cpuid_vendor(vendor);
		printf("Your cpu is %s.\n", vendor);
	}

	printf("Kernel starts at %p\n", static_cast<const void *>(&kernel_start));
	printf("Kernel ends at %p\n", static_cast<const void *>(&kernel_end));

	printf("Testing memory allocation.\n");
	void *old_mem_ptr, *mem_ptr;
	mem_results mem;
	mem=get_mem(&old_mem_ptr, 1);
	if(mem!=mem_success){kcriticalf("Getting memory returned %u.", mem); abort();}
	printf("Getting mem area returns %d.\n", mem);
	printf("Got a page at %p.\n", old_mem_ptr);
	mem=free_mem(old_mem_ptr, 1);
	if(mem!=mem_success){kcriticalf("Getting memory returned %d.", mem); abort();}
	printf("Freeing an allocated area returns %d.\n", mem);
	mem=get_mem(&mem_ptr, 1);
	if(mem!=mem_success){kcriticalf("Getting memory returned %d.", mem); abort();}
	printf("Getting mem area returns %d.\n", mem);
	printf("Got another page at %p.\n", mem_ptr);
	if(mem_ptr==old_mem_ptr){
		printf("Addresses match!\n");
	}
	else{
		printf("Addresses do not match!\n");
	}

	extern framebuffer fb; //the framebuffer is setup
	//Fill each corner with a color
	pixel_t p={255, 255, 255}; //white
	uint16_t maxX, maxY;
	fb.getMax(&maxX, &maxY); //get the maximum sizes
	//If it exists
	if(maxX!=0 && maxY!=0){
		fb.putRect(0,      0,      maxX/2,   maxY/2, p); //upper left
		p={255, 0, 0}; //red
		fb.putRect(maxX/2, 0,      maxX/2-1, maxY/2, p); //uppper right
		p={0, 255, 0}; //green
		fb.putRect(0,      maxY/2, maxX/2,   maxY/2-1, p); //lower left
		p={0, 0, 255}; //blue
		fb.putRect(maxX/2, maxY/2, maxX/2-1, maxY/2-1, p); //lower right
	}
	printf("Testing an syscall...\n");
	printf("It returned %ld.\n", syscall(0));
	kcritical("Nothing to do... Pausing now."); //boot.S should hang if we return
}

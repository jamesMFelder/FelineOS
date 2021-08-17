#include <stdio.h>

#include <stdlib.h>

#include <kernel/arch.h>

#include <kernel/tty.h>

#include <kernel/log.h>

#include <kernel/cpuid.h>

#include <kernel/multiboot.h>

#include <drivers/serial.h>

//Setup by the linker to be at the start and end of the kernel.
extern unsigned int kernel_start;
extern unsigned int kernel_end;

void kernel_main(multiboot_info_t *mbp, unsigned int magic) {
	char vendor[13];
	init_serial();
	boot_setup();

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

	printf("%s\n", "Sizes:");
	printf("%s: 0x%zu\n", "void *", sizeof(void*));
	printf("%s: 0x%zu\n", "unsigned int", sizeof(unsigned int));

	if(!(mbp->flags >> 6 & 0x1)){
		kcritical("No valid memory map.");
		kcritical("In grub, type 'c' for a command line and then type 'displaysmem' or 'lsmmap' to see the memory it thinks exists.");
		abort();
	}

	terminal_setcolor(color_bad_dark);
	kcritical("Nothing to do... Pausing now."); //boot.S should hang if we return
	terminal_setcolor(color_normal_light);
}

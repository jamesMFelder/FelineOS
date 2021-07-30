#include <stdio.h>

#include <kernel/arch.h>

#include <kernel/tty.h>

#include <kernel/log.h>

#include <kernel/cpuid.h>

void kernel_main(void) {
	char vendor[13];
	boot_setup();

	//Announce that we are loaded
	terminal_setcolor(color_ok_dark);
	klog("Hello kernel world!");
	terminal_setcolor(color_normal_light);

	if(cpuid_supported()){
		cpuid_vendor(vendor);
		printf("Your cpu is %s.\n", vendor);
		printf("It supports %x queries.\n", cpuid_max());
		printf("It supports %X queries.\n", cpuid_max());
		printf("It supports %d queries.\n", cpuid_max());
	}

	klog("Testing interrupts with divide by 0.");
	int i=0;
	printf("%d", 4/i);

	kcritical("Nothing to do... Pausing now."); //boot.S should hang if we return
}

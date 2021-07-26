#include <stdio.h>

#include <kernel/tty.h>

#include <kernel/log.h>

#include <kernel/cpuid.h>

void kernel_main(void) {
	char vendor[13];
	terminal_initialize();
	klog("Hello kernel world!");
	if(cpuid_supported()){
		cpuid_vendor(vendor);
		printf("Your cpu is %s.\n", vendor);
		printf("It supports %x queries.\n", cpuid_max());
		printf("It supports %X queries.\n", cpuid_max());
		printf("It supports %d queries.\n", cpuid_max());
	}
	kerror("Pausing now.\n");
}

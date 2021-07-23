#include <stdio.h>

#include <kernel/tty.h>

#include <kernel/log.h>

void kernel_main(void) {
	terminal_initialize();
	klog("Hello kernel world!");
	kerror("Pausing now.\n");
}

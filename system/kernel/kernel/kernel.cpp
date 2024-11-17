/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2023 James McNaughton Felder */

#include <cinttypes>
#include <drivers/framebuffer.h>
#include <drivers/serial.h>
#include <fcntl.h>
#include <feline/logger.h>
#include <feline/settings.h>
#include <feline/str.h>
#include <feline/tests.h>
#include <kernel/arch.h>
#include <kernel/asm_compat.h>
#include <kernel/backtrace.h>
#include <kernel/log.h>
#include <kernel/mem.h>
#include <kernel/misc.h>
#include <sys/syscall.h>
#include <unistd.h>
#ifdef __i386__
#include <kernel/cpuid.h>
#endif

/* Setup by the linker to be at the start and end of the kernel. */
extern const char kernel_start;
extern const char kernel_end;

/* The libFeline tests */
KVector<test_func, KGeneralAllocator<test_func>> test_functions;

ASM void kernel_main();

#include <kernel/vtopmem.h>

void kernel_main() {
	boot_setup();

	if (Settings::Misc::commandline) {
		kLog() << "Commandline: "
			   << strDebug(Settings::Misc::commandline.get());
	}

#ifdef __i386__
	if (cpuid_supported()) {
		unsigned char vendor[13];
		cpuid_vendor(vendor);
		kLog() << "Your cpu is " << vendor;
	}
#endif

	kLog() << "Kernel starts at " << ptr(&kernel_start);
	kLog() << "Kernel ends at " << ptr(&kernel_end);

	extern framebuffer fb; /* the framebuffer is setup */
	/* Fill each corner with a color */
	pixel_t p = {255, 255, 255}; /* white */
	uint16_t maxX, maxY;
	fb.getMax(&maxX, &maxY); /* get the maximum sizes */
	/* If it exists */
	if (maxX != 0 && maxY != 0) {
		fb.putRect(0, 0, maxX / 2, maxY / 2, p);            /* upper left */
		p = {255, 0, 0};                                    /* red */
		fb.putRect(maxX / 2, 0, maxX / 2 - 1, maxY / 2, p); /* upper right */
		p = {0, 255, 0};                                    /* green */
		fb.putRect(0, maxY / 2, maxX / 2, maxY / 2 - 1, p); /* lower left */
		p = {0, 0, 255};                                    /* blue */
		fb.putRect(maxX / 2, maxY / 2, maxX / 2 - 1, maxY / 2 - 1,
		           p); /* lower right */
	}

	kLog() << "Running tests:";
	for (auto &test : test_functions) {
		int result;
		kout output(log_level::log);
		output << test.name << "… ";
		result = (test.func)();
		if (result == 0) {
			output << "ok!";
		} else {
			output << "error!";
			kError() << "Test " << test.name << " failed with error "
					 << dec(result);
			return;
		}
	}

	kCritical() << "Nothing to do... Pausing now."; /* boot.S should hang if we
	                                                   return */
	return;
}

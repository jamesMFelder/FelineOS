/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2023 James McNaughton Felder */

#include <cinttypes>
#include <drivers/framebuffer.h>
#include <drivers/serial.h>
#include <drivers/timer.h>
#include <fcntl.h>
#include <feline/logger.h>
#include <feline/settings.h>
#include <feline/str.h>
#include <feline/tests.h>
#include <kernel/arch.h>
#include <kernel/asm_compat.h>
#include <kernel/backtrace.h>
#include <kernel/halt.h>
#include <kernel/log.h>
#include <kernel/mem.h>
#include <kernel/misc.h>
#include <kernel/scheduler.h>
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

void kernel_main() {
	boot_setup();

	init_scheduler();
	init_timers();

	if (Settings::Misc::commandline) {
		kLog() << "Commandline: "
			   << strDebug(Settings::Misc::commandline.get());
	}

#ifdef __i386__
	if (cpuid_supported()) {
		char vendor[13];
		cpuid_vendor(vendor);
		kLog() << "Your cpu is " << KStringView(vendor, 12);
	}
#endif

	add_new_task([]() __attribute__((noreturn)) {
		extern framebuffer fb; /* the framebuffer is setup */
		/* Fill each corner with a color */
		uint16_t maxX, maxY;
		fb.getMax(&maxX, &maxY); /* get the maximum sizes */
		/* If it exists */
		if (maxX != 0 && maxY != 0) {
			pixel_t p = {255, 255, 255};             /* white */
			fb.putRect(0, 0, maxX / 2, maxY / 2, p); /* upper left */
			sched();
			p = {255, 0, 0}; /* red */
			fb.putRect(maxX / 2, 0, maxX / 2 - 1, maxY / 2,
			           p); /* upper right */
			sched();
			p = {0, 255, 0};                                    /* green */
			fb.putRect(0, maxY / 2, maxX / 2, maxY / 2 - 1, p); /* lower left */
			sched();
			p = {0, 0, 255}; /* blue */
			fb.putRect(maxX / 2, maxY / 2, maxX / 2 - 1, maxY / 2 - 1,
			           p); /* lower right */
		}
		end_cur_task();
	});

	add_new_task([]() __attribute__((noreturn)) {
		kLog() << "Running tests:";
		for (auto &test : test_functions) {
			{
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
					break;
				}
			}
			sched();
		}
		end_cur_task();
	});

	// halt();
	end_cur_task();
}

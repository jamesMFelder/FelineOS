/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2023 James McNaughton Felder */

#include <cinttypes>
#include <cstddef>
#include <cstdio>
#include <kernel/log.h>

#include <kernel/backtrace.h>

/* Do a backtrace showing up to BT_STATIC_LEN functions */
void backtrace() {
	/* Create an array of pointers. */
	void *backtrace[BT_STATIC_LEN] = {nullptr};
	/* How far back we actuall got. */
	uint32_t stored;
	/* Actually do the backtrace */
	stored = walk_stack(backtrace, BT_STATIC_LEN);
	/* Print it */
	kerror("Here is the backtrace.");
	kerror("To get function names run addr2line -Cpfe kernel $pointer");
	for (uint32_t i = 0; i < stored && backtrace[i] != nullptr; i++) {
		printf("%p\n", backtrace[i]);
	}
}

/* Do a backtrace showing up to BT_STATIC_LEN functions starting at fp */
void backtrace_from(void *fp) {
	/* Create an array of pointers. */
	void *backtrace[BT_STATIC_LEN] = {nullptr};
	/* How far back we actuall got. */
	uint32_t stored;
	/* Actually do the backtrace */
	stored = walk_stack_from(backtrace, BT_STATIC_LEN, fp);
	/* Print it */
	kerror("Here is the backtrace.");
	kerror("To get function names run addr2line -Cpfe kernel $pointer");
	for (uint32_t i = 0; i < stored && backtrace[i] != nullptr; i++) {
		printf("%p\n", backtrace[i]);
	}
}

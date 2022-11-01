/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2021 James McNaughton Felder */

#include <cstddef>
#include <cinttypes>
#include <kernel/log.h>
#include <cstdio>

#include <kernel/backtrace.h>

/* Do a backtrace showing up to BT_STATIC_LEN functions */
void backtrace(){
#ifndef __arm__ //Temporary measure while arm doesn't have this function
	/* Create an array of pointers. */
	void* backtrace[BT_STATIC_LEN]={nullptr};
	/* How far back we actuall got. */
	uint32_t stored;
	/* Actually do the backtrace */
	stored=walk_stack(backtrace, BT_STATIC_LEN);
	/* Print it */
	kerror("Here is the backtrace.");
	kerror("To get function names run addr2line -Cpfe kernel $pointer");
	for(uint32_t i=0; i<stored && backtrace[i]!=nullptr; i++){
		printf("%p\n", backtrace[i]);
	}
#endif // ! __arm__
}

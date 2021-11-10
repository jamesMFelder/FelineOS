// SPDX-License-Identifier: MIT
// Copyright (c) 2021 James McNaughton Felder
#include <stdint.h>
#include <stdlib.h>
#include <kernel/log.h>

#if UINT32_MAX == UINTPTR_MAX
#define STACK_CHK_GUARD 0xe2dee396
#else
#define STACK_CHK_GUARD 0x595e9fbd94fda766
#endif

uintptr_t __stack_chk_guard = STACK_CHK_GUARD;

__attribute__((noreturn))
void __stack_chk_fail(void)
{
#if __STDC_HOSTED__
	abort();
	__builtin_unreachable();
#elif defined(__is_libk)
	kcritical("Stack smashing detected");
	__asm__ ("hlt");
	__builtin_unreachable();
#else
#error "If you aren't building hosted, build as part of libk."
#endif
}

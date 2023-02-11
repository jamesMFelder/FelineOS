/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2023 James McNaughton Felder */
#include <cstdint>
#include <cstdlib>
#include <kernel/log.h>
#include <bits/c_compat.h>

#if UINT32_MAX == UINTPTR_MAX
#define STACK_CHK_GUARD 0xe2dee396
#else
#define STACK_CHK_GUARD 0x595e9fbd94fda766
#endif

uintptr_t __stack_chk_guard = STACK_CHK_GUARD;

/* Run when the stack is corrupted. */
/* See https://wiki.osdev.org/Stack_Smashing_Protector for info about */
/* what to do here and what do avoid at all costs. */
/* TODO: make this a stub for a specific syscall */
C_LINKAGE __attribute__((noreturn)) void __stack_chk_fail(void);

void __stack_chk_fail(void)
{
#if __STDC_HOSTED__
	abort();
	__builtin_unreachable();
#else
	kcritical("Stack smashing detected");
	abort();
	__builtin_unreachable();
#endif
}

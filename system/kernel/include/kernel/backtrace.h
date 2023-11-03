/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2023 James McNaughton Felder */

#include <cstdint>
#include <kernel/asm_compat.h>

/* Since we don't have dynamic memory allocation (yet), how far back we go. */
/* Increase this if you need more functions showing. */
#define BT_STATIC_LEN 15

ASM void backtrace();
ASM void backtrace_from(void *fp);
ASM uint32_t walk_stack(void* array[], uint32_t max_len);
ASM uint32_t walk_stack_from(void* array[], uint32_t max_len, void* fp);

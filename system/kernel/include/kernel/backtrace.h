// SPDX-License-Identifier: MIT
// Copyright (c) 2021 James McNaughton Felder

#include <stdint.h>
#include <kernel/asm_compat.h>

//Since we don't have dynamic memory allocation (yet), how far back we go.
//Increase this if you need more functions showing.
#define BT_STATIC_LEN 15

void backtrace();
ASM uint32_t walk_stack(void* array[], uint32_t max_len);

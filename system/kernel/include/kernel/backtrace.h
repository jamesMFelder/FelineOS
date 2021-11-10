// SPDX-License-Identifier: MIT
// Copyright (c) 2021 James McNaughton Felder

#include <stdint.h>

typedef struct stackframe{
	struct stackframe *outer;
	void *caller;
} stackframe_t;

//Since we don't have dynamic memory allocation (yet), how far back we go.
//Increase this if you need more functions showing.
#define BT_STATIC_LEN 5

#ifdef __cplusplus
extern "C"{
#endif

void backtrace();
uint32_t walk_stack(void* array[], uint32_t max_len);

#ifdef __cplusplus
}
#endif

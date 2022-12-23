/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2021 James McNaughton Felder */
#include "idt.h"
#include <cinttypes>
#include <cstdlib>
#include <cstring>
#include <kernel/interrupts.h>
#include <kernel/log.h>

extern unsigned char* isr_source;
extern unsigned char* isr_end;
extern unsigned char* isr_dest;

void idt_init() {
	/* Copy the table to 0x0 */
	for (auto offset=0; (&isr_source+offset) < &isr_end; ++offset) {
		*(&isr_dest+offset) = *(&isr_source+offset);
	}
	//memcpy(&isr_dest, &isr_source, &isr_end-&isr_source);
}

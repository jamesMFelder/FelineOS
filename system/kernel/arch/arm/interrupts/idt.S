/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2023 James McNaughton Felder */

/* Loads the address of isr_start into the coprocessor register
 * on the Raspberry Pi 1B that sets the exception base address
 * (where the code for an interrupt/syscall/etc. will be loaded from) */
.global idt_init
idt_init:

	.extern isr_start
	ldr r0, =isr_start
	mcr p15, 0, r0, c12, c0, 0

	bx lr

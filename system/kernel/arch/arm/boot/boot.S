/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2023 James McNaughton Felder */

#include <kernel/vtopmem.h>
#include <kernel/arch/arm/modes.inc>

.section .bss, "aw"
	/* Reserve stacks all modes */
	.align 16
	fiq_stack_bottom:
		.skip 1024 /* 1 KiB */
	fiq_stack_top:
	irq_stack_bottom:
		.skip 1024 /* 1 KiB */
	irq_stack_top:
	svc_stack_bottom:
		.skip 1024 /* 1 KiB */
	svc_stack_top:
	abt_stack_bottom:
		.skip 1024 /* 1 KiB */
	abt_stack_top:
	und_stack_bottom:
		.skip 1024 /* 1 KiB */
	und_stack_top:
	sys_stack_bottom:
		.skip 16384 /* 16 KiB */
	sys_stack_top:

.section .rodata, "a"
	boot_msg:
	.asciz "Running at %p.\n"

.macro reset mode
cps #\mode\()_mode
ldr sp, =\mode\()_stack_top
eor fp, fp
.endm

/* The kernel entry point. */
.section .startup.text, "ax"
.global _start
_start:
	/* Setup the stack and clear the frame pointer so debuggers don't trace further back from here for all stacks */
	reset abt
	reset und
	reset sys
	reset irq
	reset fiq
	reset svc
	cps #sys_mode

	/* Setup what C(++) has no idea of */
	mov r0, r2
	.extern early_boot_setup
	bl early_boot_setup

	/* Call the global constructors. */
	.extern _init
	bl _init

	.extern printf
	ldr r0, =boot_msg
	mov r1, pc
	bl printf
	svc #0

	/* Transfer control to the main kernel. */
	.extern kernel_main
	bl kernel_main

	/* Call destructors (shouldn't reach here) */
	mov r0, #0
	.extern __cxa_finalize
	bl __cxa_finalize

	/* Hang if kernel_main unexpectedly returns. */
.hang:
	bl .hang

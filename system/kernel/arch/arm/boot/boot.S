/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2023 James McNaughton Felder */

#include <kernel/vtopmem.h>

/* Reserve a stack for the initial thread. */
.section .bss, "aw"
	.align 16
	stack_bottom:
		.skip 16384 /* 16 KiB */
	stack_top:

/* Reserve initial page tables */
.section .bss, "aw"
	.balign 512
	boot_page_directory:
	.skip 4096
	boot_page_table1:
	.skip 4096
	boot_page_table2:
	.skip 4096
	boot_page_table3:
	.skip 4096
	boot_page_table4:
	.skip 4096

.section .rodata, "a"
	boot_msg:
	.asciz "Running at %p.\n"

/* The kernel entry point. */
.section .startup.text, "ax"
.global _start
_start:
	/* Setup the stack and clear the frame pointer so debuggers don't trace further back from here */
	ldr sp, =stack_top
	eor fp, fp

	/* Setup what C(++) has no idea of */
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

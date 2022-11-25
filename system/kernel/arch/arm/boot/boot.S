/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2021 James McNaughton Felder */

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

.section .rodata
	.align 16
	test_str:
	.asciz "Hello world!\r\n"

/* The kernel entry point. */
.section .startup.text, "ax"
.global _start
_start:
	/* Setup the stack and clear ebp so debuggers don't trace further back from here */
	ldr sp, =stack_top
	eor fp, fp
	push { fp }
	mov fp, sp

	/* Setup what C(++) has no idea of */
	.extern early_boot_setup
	bl early_boot_setup

	/* Call the global constructors. */
	.extern _init
	bl _init

	.extern writestr_serial
	ldr r0, =test_str
	bl writestr_serial

	/* Transfer control to the main kernel. */
	.extern kernel_main
	//bl kernel_main

	/* Call destructors (shouldn't reach here) */
	mov r0, #0
	.extern __cxa_finalize
	bl __cxa_finalize

	/* Hang if kernel_main unexpectedly returns. */
	//cli
.hang:
	bl .hang

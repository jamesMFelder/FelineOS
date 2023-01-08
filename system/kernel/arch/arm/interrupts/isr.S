/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2022 James McNaughton Felder */

//All possible exceptions (will be moved to 0x0 at runtime)
.section .interrupt.text, "ax"
.global isr_stub_table
isr_stub_table:
	ldr pc,=isr_handler
	ldr pc,=isr_handler
	ldr pc,=isr_handler
	ldr pc,=isr_handler
	ldr pc,=isr_handler
	ldr pc,=isr_handler
	ldr pc,=isr_handler
	ldr pc,=isr_handler

//Handle an unknown exception. Don't do anything fancy because we can't assume anything.
.section .text
.global isr_handler
isr_handler:
	mov ip, sp
	push {fp, ip, lr, pc}
	sub fp, ip, #4
	ldr r0, =isr_msg
	mov r1, lr
	bl printf
	bl abort

.section .rodata
isr_msg:
	.asciz "Unknown exception %d called!\nThe backtrace unfortunately might show the instruction after the one that caused the exception.\n"

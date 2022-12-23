/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2022 James McNaughton Felder */

.section .interrupt.text, "ax"

//TODO: fill out table for all possible exceptions
// for now, just enough to make the svc instruction get caught
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
	ldr pc,=isr_handler
	ldr pc,=isr_handler

.section .text
.global isr_handler
isr_handler:
	//movs pc, r14 //Just jump back
	//sub lr, lr, #4 // construct the return address
	ldr r0,=isr_msg
	bl printf
	bl abort
	hlt
	ldm sp, {pc}^ //Pop the return address from the stack, and change back the mode

.section .rodata
isr_msg:
	.asciz "Interrupt called!\nThe backtrace unfortunately doesn't yet show what happened before the interrupt.\n"

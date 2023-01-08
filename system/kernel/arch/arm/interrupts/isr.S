/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2022 James McNaughton Felder */

//All possible exceptions (will be moved to 0x0 at runtime)
.section .interrupt.text, "ax"
.global isr_stub_table
isr_stub_table:
	ldr pc,=isr_handler
	ldr pc,=isr_handler
	ldr pc,=swi
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

.global swi
swi:
	//Construct a stack frame for backtracing
	//Don't save r0, because that is our return value.
	//Save everything else, because I haven't decided yet what what needs to be saved
	mov ip, sp
	push {fp, ip, lr, pc}
	sub fp, ip, #4
	push {r1, r2}

	//Get the instruction that called the interrupt
	// (not the one we will return to).
	mov r1, lr
	sub r1, #4

	//Get the interrupt number
	//TODO: should we always expect 0, or should we use this as the actual system call selector
	ldr r2, [r1]
	and r2, #(1<<24)-1

	//Print an error message and backtrace
	ldr r0,=swi_msg
	bl printf
	bl backtrace

	//Return success
	mov r0, #0

	//Remove the stack frame and return
	pop {r1, r2}
	sub sp, fp, #12
	ldm sp, {fp, sp, lr}
	bx lr

.section .rodata
isr_msg:
	.asciz "Unknown exception %d called!\nThe backtrace unfortunately might show the instruction after the one that caused the exception.\n"
swi_msg:
	.asciz "Instruction %p made system call %u. Aborting now.\nNote that the backtrace will show the instruction after the one that made the system call.\n"

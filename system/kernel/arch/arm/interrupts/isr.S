/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2023 James McNaughton Felder */

#include <kernel/arch/arm/modes.inc>

//All possible exceptions
.section .interrupt.text, "ax"
.align 5
.global isr_stub_table
isr_stub_table:
	ldr pc,=isr_handler
	ldr pc,=undef_handler
	ldr pc,=swi
	ldr pc,=isr_handler
	ldr pc,=data_abort_handler
	ldr pc,=isr_handler
	ldr pc,=isr_handler
	ldr pc,=isr_handler

//Handle an unknown exception. Don't do anything fancy because we can't assume anything.
.section .text
.global isr_handler
isr_handler:
	//Save all the registers for printing later
	push {r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12}

	//Print what happened
	subs r1, r14, #8
	mrs r2, cpsr
	and r2, r2, #0b11111
	ldr r0,=isr_msg
	bl printf

	//Print all the registers
	pop {r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12}
	push {r3, r4, r5, r6, r7, r8, r9, r10, r11, r12}
	mov r3, r2
	mov r2, r1
	mov r1, r0
	ldr r0, =register_dump_msg
	bl printf

	//Print backtrace and hang
	bl abort

// Handle an undefined instruction
.global undef_handler
undef_handler:
	srsfd und_mode
	push {r0, r1, r2, r3}

	ldr r0, =undef_msg
	sub r2, lr, #4
	ldr r1, [r2]
	bl printf

	// Get the user frame pointer (for backtracing)
	cps sys_mode
	mov r0, fp
	cps und_mode
	.extern backtrace_from
	bl backtrace_from

	bl abort
	rfefd sp!

.global data_abort_handler
data_abort_handler:

	// Save exception state and preserve registers, creating a new stack frame
	clrex
	push {fp, ip, lr}
	add fp, sp, #0xc
	push {r0, r1, r2, r3}

	//Get the faulting address
	sub r0, lr, #8

	//Get fault status register (from https://developer.arm.com/documentation/ddi0198/e/I1039321)
	mrc p15, 0, r1, c5, c0, 0 //read DFSR

	//Get the fault address register (from https://developer.arm.com/documentation/ddi0211/k/system-control-coprocessor/system-control-coprocessor-register-descriptions/c6--fault-address-register--far)
	mrc p15, 0, r2, c6, c0, 0 //read FAR

	//Deal with this in c++ (doesn't return on an unrecoverable abort)
	.extern handle_abort
	bl handle_abort

	// Restore pre-exception state
	pop {r0, r1, r2, r3}
	sub fp, sp, #0xc
	pop {fp, ip, lr}
	subs pc, lr, #4

.global swi
swi:
	//Push lr and spsr to the stack
	srsfd #sys_mode!
	//Switch to system mode so we can find out what we're being asked to do
	cps #sys_mode
	//Construct a stack frame for backtracing
	//Don't save r0, because that is our return value.
	//Save everything else, because I haven't decided yet what what needs to be saved
	push {fp, ip, lr}
	add fp, sp, #0xc

	//Get the instruction that called the interrupt
	// store the one we will return to, and use the one that called us
	ldr r1, [fp]
	str r1, [fp, #-4]
	sub r1, r1, #4

	//Get what the instruction is (r3) and the interrupt number (r2)
	//TODO: should we always expect 0, or should we use this as the actual system call selector
	ldr r3, [r1]
	and r2, r3, #(1<<24)-1

	//Print an error message and backtrace
	ldr r0,=swi_msg
	bl printf

	bl backtrace

	//Return success
	sub sp, fp, #0xc
	pop {fp, ip, lr}
	mov r0, #0

	//Remove the stack frame and return
	rfefd sp!

.section .rodata
isr_msg:
	.asciz "Unknown exception called occured at instruction %p! Mode is now %#b.\n"
register_dump_msg:
	.asciz "r0: %#x\nr1: %#x\nr2: %#x\nr3: %#x\nr4: %#x\nr5: %#x\nr6: %#x\nr7: %#x\nr8: %#x\nr9: %#x\nr10: %#x\nr11: %#x\nr12: %#x\n"
undef_msg:
	.asciz "Undefined instruction (%#x) executed at %p.\n"
swi_msg:
	.asciz "Instruction %p made system call %u. The instruction is %#x.\n"

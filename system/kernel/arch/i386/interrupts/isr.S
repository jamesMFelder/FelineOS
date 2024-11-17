/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2023 James McNaughton Felder */
.extern kcriticalf /* Log an critical error printf(3) style */
.extern kerrorf /* Log an error printf(3) style */
.extern klogf /* Logging printf(3) style */
.extern abort

.macro isr_err_stub isr_num

isr_stub_\isr_num:
	push %ebp /* Create a stack frame for debugging */
	mov  %esp, %ebp

    push %eax /* Save registers */
	push %ecx
	push %edx
	cld /* Reset string operation direction flag before calling C code */

	push 8(%ebp) /* Push the error code for the logging function */
    push \isr_num /* Push the error code for the logging function */
	push $error_msg_generic_interrupt
    call kerrorf /* Error because we don't know what to do */

	push $interrupt_corrupted_stack_trace /* Apologize for the broken stack trace */
	call kerrorf

	push 4(%ebp) /* Push the faulting instruction */
	push $error_msg_generic_eip
	call kerrorf

	hlt /* Halt because we don't know what to do */

	add $12, %esp /* Get rid of local variables */

    pop %edx /* Restore registers we pushed */
	pop %ecx
	pop %eax
	mov %esp, %ebp
	pop %ebp
    add $4, %esp /* Restore the stack to how it was before the interrupt */
    iret  /* Return from the interrupt */
.endm
/* if writing for 64-bit, use iretq instead */

.macro isr_no_err_stub isr_num
isr_stub_\isr_num:
	push %ebp /* Create a stack frame for debugging */
	mov  %esp, %ebp

    push %eax /* Save registers */
	push %ecx
	push %edx

    push $ \isr_num /* Add the exeption number to the stack */
	push $noerror_msg_generic_interrupt
    call kerrorf /* Error because we don't know what to do */

	push 4(%ebp) /* Push the faulting instruction */
	push $error_msg_generic_eip
	call kerrorf

	add $8, %esp /* Remove local variables */

    hlt /* Halt because we don't know what to do */

    pop %edx /* Restore registers */
	pop %ecx
	pop %eax

	mov %ebp, %esp /* Remove the stack frame */
	pop %ebp

    iret /* Return from the interrupt */
.endm

.section .text
/* List of interrupt service routines */
/* #DE (Division by Zero) */
isr_stub_0:
	push %ebp /* Create a stack frame (for debugging) */
	mov  %esp, %ebp
	push %eax /* Save registers */
	push %ecx
	push %edx
	cld /* Reset string operation direction flag before calling C code */
	push 4(%ebp) /* put the fauling address lower on the stack (for logging) */
	/* Change to `push QWORD [ebp+8]` for 64-bit */
	push $error_msg_division_by_zero /* The format string */
	call kerrorf
	call abort /* Hang */
isr_no_err_stub 1
isr_no_err_stub 2
isr_no_err_stub 3
isr_no_err_stub 4
isr_no_err_stub 5
isr_no_err_stub 6
isr_no_err_stub 7
/* #DF (Double fault) */
/* 99% unrecoverable */
isr_stub_8:
	push %ebp /* Create a stack frame (for debugging) */
	mov  %esp, %ebp
	push %eax /* Save registers */
	push %ecx
	push %edx
	cld /* Reset string operation direction flag before calling C code */
	push $error_msg_double_fault
	call kcriticalf
	push $interrupt_corrupted_stack_trace /* Apologize for the broken stack trace */
	call kcriticalf
	call abort /* Hang */
isr_no_err_stub 9
isr_err_stub    10
isr_err_stub    11
isr_err_stub    12
/* #GPF(General Protection Fault) */
isr_stub_13:
	push %ebp /* Create a stack frame (for debugging) */
	mov  %esp, %ebp
	push %eax /* Save registers */
	push %ecx
	push %edx

	cld /* Reset string operation direction flag before calling C code */

	push 8(%ebp) /* put the fauling address lower on the stack (for logging) */
	/* Change to `push QWORD [ebp+8]` for 64-bit */
	push $error_msg_GPF /* The format string */
	call kerrorf

	mov 4(%ebp), %eax /* Check if we have a relevant segment selector index */
	cmp $0,      %eax
	je gpf_end /* If we don't (it's 0), jump to the abort call */

	push %eax /* put the segment selector index on the stack (for logging) */
	push $error_msg_GPF_segment /* Format string if we have a segment selector index */
	call kerrorf

	push $interrupt_corrupted_stack_trace /* Apologize for the broken stack trace */
	call kerrorf

	gpf_end: /* Jump here to skip printing a non-relevant segment selector index */
	call abort /* Hang */
/* #PF (Page Fault) */
isr_stub_14:
	push %ebp /* Create a stack frame */
	mov  %esp, %ebp
	push %eax /* Save registers */
	push %ecx
	push %edx
	cld  /* Reset string operation direction flag before calling C code */

	mov  %cr2, %eax
	push %eax /* Push the invalid address */

	/* Change to `push 16(%rbp)` for 64-bit */
	push 8(%ebp) /* Push the faulting address */

	mov 4(%ebp), %eax /* Check if this is due to a missing page, or invalid access to a page */
	and $1,       %eax
	cmp $0,       %eax
	jne push_invalid_access /* The page table is present */

	push $log_msg_page_notpresent /* The page table isn't present */
	jmp pg_msg_pushed /* Don't push the other message */

	push_invalid_access: /* The page table is present */
	push $log_msg_page_access_violation

	pg_msg_pushed:
	call kerrorf /* Actually print it */
	push $interrupt_corrupted_stack_trace /* Apologize for the broken stack trace */
	call kerrorf
	call abort /* We didn't actually fix anything */

	mov %ebp, %esp
	pop %ebp
	iret
isr_no_err_stub 15
isr_no_err_stub 16
isr_err_stub    17
isr_no_err_stub 18
isr_no_err_stub 19
isr_no_err_stub 20
isr_no_err_stub 21
isr_no_err_stub 22
isr_no_err_stub 23
isr_no_err_stub 24
isr_no_err_stub 25
isr_no_err_stub 26
isr_no_err_stub 27
isr_no_err_stub 28
isr_no_err_stub 29
isr_err_stub    30
/* System call (for now) */
isr_stub_31:
	push %ebp /* Create a stack frame (for debugging) */
	mov  %esp, %ebp

	cld /* Reset string operation direction flag before calling C code */

	/* Setup arguments for c++ */
	push %edx
	push %ecx
	push %eax

	call syscall_handler

	/* Don't restore stack so we preserve the return value in %eax */
	add $0xc, %esp

	mov %ebp, %esp/* Remove the stack frame */
	pop %ebp

	iret /* Return to the next instruction */

/* PIC interrupt */
isr_stub_32:
	push %ebp /* Create a stack frame for debugging */
	mov  %esp, %ebp

    push %eax /* Save registers */
	push %ecx
	push %edx

	call PIT_isr_handler

    pop %edx /* Restore registers */
	pop %ecx
	pop %eax

	mov %ebp, %esp /* Remove the stack frame */
	pop %ebp

    iret /* Return from the interrupt */


/* Used for the lidt instruction */
.section .data
.global isr_stub_table
isr_stub_table:
.irp i,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32
	.long isr_stub_\i /* use DQ instead if targeting 64-bit */
.endr

.section .bss
in_page_fault: .long 0

.section .rodata
/* Change %lX to %llX for 64-bit */
	interrupt_corrupted_stack_trace: .asciz "Replace the address that caused the fault in the stack trace below with the address shown here when you are debugging."
	error_msg_generic_interrupt: .asciz "Unknown interrupt 0x%lX fired with error 0x%lX."
	noerror_msg_generic_interrupt: .asciz "Unknown interrupt 0x%lX fired"
	error_msg_generic_eip: .asciz "The instruction was at %p."
	error_msg_division_by_zero: .asciz "Division by zero at %p."
	error_msg_double_fault: .asciz "A double fault occured. Halting."
	error_msg_GPF: .asciz "General protection fault at %p."
	error_msg_GPF_segment: .asciz "Segment selector index: 0x%lX."
	log_msg_page_notpresent: .asciz "Instruction at %p attempted to access unmapped page at %p."
	log_msg_page_access_violation: .asciz "Instruction at %p is not allowed to access to address %p."

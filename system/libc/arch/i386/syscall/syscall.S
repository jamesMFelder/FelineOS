/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2021 James McNaughton Felder */

/* Make a system call */
/* Note that any function can do this, so we don't do security, just convenience */
.global syscall

syscall:
	push %ebp /* Create a stack frame (for debugging) */
	mov %esp, %ebp

	mov 8(%ebp), %eax /* Move the first argument to eax (to select the syscall) */

	cmpl $0, %eax /* Compare against the max syscall number (0 because we don't support any) */
	jl valid
	movl $-1, %eax /* Return -1 */
	jmp done /* Jump to cleanup */
	valid:

	int $0x1F /* Call the kernel */
	/* Return value is in eax, where it should be */

	done: /* The syscall is over (used for errors in arguments) */

	mov %ebp, %esp /* Remove the stack frame */
	pop %ebp

	ret /* Return to the calling function */

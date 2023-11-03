/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2023 James McNaughton Felder */

/* Make a system call */
.global raw_syscall

raw_syscall:
	push %ebp /* Create a stack frame (for debugging) */
	mov %esp, %ebp

	mov 8(%ebp), %eax /* Move the first argument to eax (to select the syscall) */
	mov 12(%ebp), %ebx /* Pointer to arguments for syscall */
	mov 16(%ebp), %ecx /* Pointer to results for syscall */

	int $0x1F /* Call the kernel */

	mov %ebp, %esp /* Remove the stack frame */
	pop %ebp

	ret /* Return to the calling function */

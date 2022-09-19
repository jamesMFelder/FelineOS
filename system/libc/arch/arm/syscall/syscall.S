/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2021 James McNaughton Felder */

/* Make a system call */
/* Note that any function can do this, so we don't do security, just convenience */
.global syscall

syscall:
	push {fp, lr} /* Create a stack frame (for debugging) */

	cmp r0, #0 /* Compare against the max syscall number (0 because we don't support any) */
	movlt r0, #-1 /* Return -1 for an illegal syscall. */
	blt done /* Jump to cleanup if it is an illegal syscall */

	svc $0 /* Call the kernel */
	/* Return value is in r0, where it should be */

	done: /* The syscall is over (used for errors in arguments) */

	pop {fp, lr} /* Remove the stack frame */

	bx lr /* Return to the calling function */

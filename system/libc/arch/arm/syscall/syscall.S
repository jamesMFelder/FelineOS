/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2023 James McNaughton Felder */

/* Make a system call */
/* Pointer and return value stay in r0 */
.global raw_syscall
raw_syscall:
	push {fp, ip, lr}
	add fp, sp, #0xc
	svc #0
	sub sp, fp, #0xc
	pop {fp, ip, lr}
	bx lr

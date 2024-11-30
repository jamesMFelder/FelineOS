/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2024 James McNaughton Felder */

/* Store all registers and then load the new task's. */
.global swap_task_registers
swap_task_registers:
	.do_swap:
	stm r0, {r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12, r13, r14}
	ldm r1, {r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12, r13, r14}

	.switched:
	bx lr

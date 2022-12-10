/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2021 James McNaughton Felder */

/* Walks backwards through the call stack and builds a list of return addresses. */
/* Args: */
/*  * Array of 32-bit addresses (in r0). */
/*  * Maximum number of elements in array (in r1). */
/* Return value: The number of addresses stored in the array. */
/* Calling convention: eabi */
.global walk_stack

walk_stack:
	/* Create a stack frame so our caller (backtrace()) shows up in the stack). */
	mov ip, sp
	push {fp, ip, lr, pc}
	sub fp, ip, #4
	push {r4, r5, r6}

	/* Set up local registers. */
	mov r4, r0 /* r4 = destination array pointer */
	eor  r0, r0 /* r0 = return value (number of stack frames found). */
	/* Maximum array size in r1. */
	mov r5, fp /* current frame pointer in r5 */
	/* current return addr in r6 */

.walk:
	/* Walk backwards through fp linked list, storing return addresses in r4 array. */
	cmp r1, r0 /* End if we run out of space. */
	beq .done
	cmp r5, #0 /* Also end if we hit the top of the stack (the frame pointer is null). */
	beq .done

	/* Add the address to the array. */
	ldr r6, [r5, #-4]
	str r6, [r4]

	add r0, #1 /* Increment the number of entries added. */
	add r4, #4 /* Update the position in the array. */
	ldr r5, [r5, #-12] /* Get the new stack frame pointer. */
	b .walk /* Repeat if r5 isn't zero */

.done:
	/* Clear our stack frame and return. */
	push {r4, r5, r6}
	sub sp, fp, #12
	ldm sp, {fp, sp, lr}
	bx lr

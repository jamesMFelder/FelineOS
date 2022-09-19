/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2021 James McNaughton Felder */

.global enable_paging

enable_paging:
	push %ebp
	mov %esp, %ebp

	/* Load the page directory into cr3 */
	mov 8(%ebp), %eax
	mov %eax, %cr3

	/* Enable paging */
	mov %cr0, %ebx
	or  $(1<<31), %ebx
	mov %ebx, %cr0

	/* Now reload the segment registers (CS, DS, SS, etc.) with the appropriate segment selectors... */
	mov $0x10, %ax/* Data selector */
	mov %ax, %ds
	mov %ax, %es
	mov %ax, %fs
	mov %ax, %gs

	/* Reload CS with the code selector */
	/* Why AT&T! This is jmp 0x08:reloadCS!? */
	jmp $0x08, $reloadCS /* Code selector */

	reloadCS:
	mov $0, %eax
	mov %ebp, %esp
	pop %ebp
	ret

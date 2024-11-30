/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2024 James McNaughton Felder */

.global get_eflags
get_eflags:
	/* Create a stack frame */
	push %ebp
	mov %esp, %ebp

	/* push eflags onto the stack */
	pushf
	/* Pop it from the stack to eax (return value) */
	pop %eax

	/* Clear the stack frame and return */
	mov %ebp, %esp
	pop %ebp
	ret

/* Create a fake stack frame at arg1 which returns to arg2 "passing it" a value of arg3.
 * After this the stack should look like:
 * - stack[0]:   "argument" to "caller" function
 * - stack[-4]:  0 - fake return address to make the caller function find the argument
 * - stack[-8]:  address to "return" to at the start of the caller function
 * - stack[-12]: 0 - empty frame pointer to mark the top of the stack
*/
.global init_frame
init_frame:
	/* Create a stack frame */
	push %ebp
	mov %esp, %ebp

	/* Use the in-construction stack */
	mov 8(%ebp), %esp
	/* Push the argument for the 'caller' function */
	push 16(%ebp)
	/* Push a false 'return-address' to add the right spacing to the stack. */
	push $0
	/* Push the fake return address */
	push 12(%ebp)
	/* Create a fake stack frame */
	push $0

	/* Return the value of the current stack pointer */
	mov %esp, %eax

	/* Clear the stack frame and return */
	mov %ebp, %esp
	pop %ebp
	ret

/* Store all registers and then load the new task's. The current task's
 * function to return to is stored on the stack (and thus saved by esp and ebp).
 * Since everything is loaded/saved using absolute offsets from eax/ebx, storing
 * ebx can happen at any time, but loading eax must happen last.
 */
.global swap_task_registers
swap_task_registers:
	/* Create a stack frame for the old task */
	push %ebp
	mov %esp, %ebp

	/* Get the arguments: eax=load, ebx=save */
	mov 8(%ebp), %ebx
	mov 12(%ebp), %eax

	/* Save the general purpose registers */
	mov %eax, 0x00(%ebx)
	mov %ebx, 0x04(%ebx)
	mov %ecx, 0x08(%ebx)
	mov %edx, 0x0c(%ebx)
	mov %esi, 0x10(%ebx)
	mov %edi, 0x14(%ebx)
	mov %ebp, 0x18(%ebx)
	mov %esp, 0x1c(%ebx)

	/* Get the eflags register, and save it */
	pushf
	pop %ecx
	mov %ecx, 0x20(%ebx)

	/* Load the eflags register */
	mov 0x20(%eax), %ecx
	push %ecx
	popf

	/* Load the general purpose registers */
	mov 0x1c(%eax), %esp
	mov 0x18(%eax), %ebp
	mov 0x14(%eax), %edi
	mov 0x10(%eax), %esi
	mov 0x0c(%eax), %edx
	mov 0x08(%eax), %ecx
	mov 0x04(%eax), %ebx
	mov 0x00(%eax), %eax

	/* Clear the stack frame from the new task */
	mov %ebp, %esp
	pop %ebp

	ret

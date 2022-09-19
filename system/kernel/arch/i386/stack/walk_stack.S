/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2021 James McNaughton Felder */

/* Walks backwards through the call stack and builds a list of return addresses. */
/* Args: */
/*  * Array of 32-bit addresses. */
/*  * Maximum number of elements in array. */
/* Return value: The number of addresses stored in the array. */
/* Calling convention: cdecl */
.global walk_stack

walk_stack:
    /* Create stack frame & save caller's EDI and EBX. */
    push %ebp
    mov  %esp,   %ebp
	push %edi
	push %ebx

    /* Set up local registers. */
    xor  %eax,       %eax    /* EAX = return value (number of stack frames found). */
    mov  (%ebp),     %ebx    /* EBX = old EBP. */
    mov  8(%ebp),    %edi    /* Destination array pointer in EDI. */
    mov  12(%ebp),   %ecx    /* Maximum array size in ECX. */

.walk:
    /* Walk backwards through EBP linked list, storing return addresses in EDI array. */
    test %ebx,   %ebx
    jz   .done
    mov  4(%ebx),   %edx     /* EDX = previous stack frame's IP. */
    mov  0(%ebx),   %ebx     /* EBX = previous stack frame's BP. */
	sub  $1,        %edx     /* We got the IP of the next instruction */
    mov  %edx,      (%edi)   /* Copy IP to the array. */
    add  $4,         %edi
    inc  %eax
    loop .walk /* Repeat if ecx isn't zero */

.done:
    /* Restore caller's EDI and EBX, leave stack frame & return EAX. */
    pop %ebx
    pop %edi
    mov %ebp, %esp
	pop %ebp
    ret

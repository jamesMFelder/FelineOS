/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2023 James McNaughton Felder */
.global setGdt

gdtr:
	.int  0 /* For limit storage */
	.long 0 /* For base storage */

setGdt:
   cli /* Disable interrupts */
   mov   4(%esp), %eax
   mov   %eax, (gdtr+2)
   mov   8(%esp), %ax
   dec   %ax
   mov   %ax, (gdtr)
   lgdt  (gdtr)

   /* Reload CS register containing code selector: */
   JMP   $0x08, $.reload_CS /* 0x08 points at the new privliged code selector */


.reload_CS:
   /* Reload data segment registers: */
   mov $0x10, %ax /* 0x10 points at the new privliged data selector */
   mov %ax,   %ds
   mov %ax,   %es
   mov %ax,   %fs
   mov %ax,   %gs
   mov %ax,   %ss
   ret

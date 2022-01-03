// SPDX-License-Identifier: MIT
// Copyright (c) 2021 James McNaughton Felder
// Declare constants for the multiboot header.
.set MBALIGN,  1<<0             // align loaded modules on page boundaries
.set MEMINFO,  1<<1             // provide memory map
.set VIDINFO,  1<<2             // we have a preference about the graphics mode
.set FLAGS,    MBALIGN | MEMINFO | VIDINFO // this is the Multiboot 'flag' field
.set MAGIC,    0x1BADB002       // 'magic number' lets bootloader find the header
.set CHECKSUM, -(MAGIC + FLAGS) // checksum of above, to prove we are multiboot

.set HEADER_ADDR,   0 // ELF contains this info
.set LOAD_ADDR,     0 // ELF contains this info
.set LOAD_END_ADDR, 0 // ELF contains this info
.set BSS_END_ADDR,  0 // ELF contains this info
.set ENTRY_ADDR,    0 // ELF contains this info

.set GMODE,  0 // framebuffer instead of text mode
.set WIDTH,  0 // no preference about pixels or characters per line
.set HEIGHT, 0 // no preference about pixels or characters per column
.set DEPTH,  0 // no preference about bits per pixel

// Declare a header as in the Multiboot Standard.
.section .multiboot
	.align 4
	.long MAGIC
	.long FLAGS
	.long CHECKSUM

	.long HEADER_ADDR
	.long LOAD_ADDR
	.long LOAD_END_ADDR
	.long BSS_END_ADDR
	.long ENTRY_ADDR

	.long GMODE
	.long WIDTH
	.long HEIGHT
	.long DEPTH

// Reserve a stack for the initial thread.
.section .bss
	.align 16
	stack_bottom:
		.skip 16384 // 16 KiB
	stack_top:

// The kernel entry point.
.section .text
.global _start
_start:
	//Setup the stack and clear ebp so debuggers don't trace further back from here
	mov $stack_top, %esp
	xor %ebp, %ebp
	push %ebp
	mov %esp, %ebp

	//Save the arguments from GRUB
	push %eax
	push %ebx

	// Call the global constructors.
	.extern _init
	call _init

	// Transfer control to the main kernel.
	.extern kernel_main
	call kernel_main

	//Call destructors (shouldn't reach here)
	sub $4, %esp
	movw $0, (%esp)
	.extern __cxa_finalize
	call __cxa_finalize
	add $4, %esp

	// Hang if kernel_main unexpectedly returns.
	cli
.hang:
	hlt
	jmp .hang

.end:

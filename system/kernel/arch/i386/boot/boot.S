/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2021 James McNaughton Felder */
/* Declare constants for the multiboot header. */
.set MBALIGN,  1<<0             /* align loaded modules on page boundaries */
.set MEMINFO,  1<<1             /* provide memory map */
.set VIDINFO,  1<<2             /* we have a preference about the graphics mode */
.set FLAGS,    MBALIGN | MEMINFO | VIDINFO /* this is the Multiboot 'flag' field */
.set MAGIC,    0x1BADB002       /* 'magic number' lets bootloader find the header */
.set CHECKSUM, -(MAGIC + FLAGS) /* checksum of above, to prove we are multiboot */

.set HEADER_ADDR,   0 /* ELF contains this info */
.set LOAD_ADDR,     0 /* ELF contains this info */
.set LOAD_END_ADDR, 0 /* ELF contains this info */
.set BSS_END_ADDR,  0 /* ELF contains this info */
.set ENTRY_ADDR,    0 /* ELF contains this info */

.set GMODE,  0 /* framebuffer instead of text mode */
.set WIDTH,  0 /* no preference about pixels or characters per line */
.set HEIGHT, 0 /* no preference about pixels or characters per column */
.set DEPTH,  0 /* no preference about bits per pixel */

#include <kernel/vtopmem.h>

// Declare a header as in the Multiboot Standard.
.section .multiboot.data, "aw"
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

/* Reserve a stack for the initial thread. */
.section .bss, "aw"
	.align 16
	stack_bottom:
		.skip 16384 /* 16 KiB */
	stack_top:

/* Reserve initial page tables */
.section .bss, "aw"
	.align 4096
	boot_page_directory:
	.skip 4096
	boot_page_table1:
	.skip 4096
	boot_page_table2:
	.skip 4096
	boot_page_table3:
	.skip 4096
	boot_page_table4:
	.skip 4096

/* The kernel entry point. */
.section .multiboot.text, "a"
.global _start
.type _start, @function
_start:
	//Get the physical address of boot_page_table1
	movl $(boot_page_table1 - VA_OFFSET), %edi
	//Start mapping at address 0
	movl $0, %esi
	//Map 4096 pages
	movl $4096, %ecx

1:
	//Only map the kernel
	cmpl $identity_map_from_here, %esi
	jl 2f
	cmpl $phys_kernel_end, %esi
	jge 3f

	//Map the addresses as "present, writeable". Not secure - get rid of quick
	movl %esi, %edx
	orl $0x003, %edx
	movl %edx, (%edi)

2:
	//Each page is 4096 bytes
	addl $4096, %esi
	//Size of each of the entries is 4 bytes
	addl $4, %edi
	//Loop to the next entry if we haven't finished
	loop 1b

3:
	//Map the page table to virtual addresses 0x00000000 (identity map)/* and VA_OFFSET (higher-half)*/
	movl $(boot_page_table1 - VA_OFFSET + 0x003), boot_page_directory - VA_OFFSET + 0
	movl $(boot_page_table2 - VA_OFFSET + 0x003), boot_page_directory - VA_OFFSET + 0 + 4
	movl $(boot_page_table3 - VA_OFFSET + 0x003), boot_page_directory - VA_OFFSET + 0 + 8
	movl $(boot_page_table4 - VA_OFFSET + 0x003), boot_page_directory - VA_OFFSET + 0 + 12

	/*movl $(boot_page_table1 - VA_OFFSET + 0x003), boot_page_directory - VA_OFFSET + 768*4
	movl $(boot_page_table2 - VA_OFFSET + 0x003), boot_page_directory - VA_OFFSET + 768*4 + 4
	movl $(boot_page_table3 - VA_OFFSET + 0x003), boot_page_directory - VA_OFFSET + 768*4 + 8
	movl $(boot_page_table4 - VA_OFFSET + 0x003), boot_page_directory - VA_OFFSET + 768*4 + 12*/

	//Set cr3 to the address of the boot_page_directory
	movl $(boot_page_directory - VA_OFFSET), %ecx
	movl %ecx, %cr3

	//Enable paging and the write-protect bit
	movl %cr0, %ecx
	orl $0x80010000, %ecx
	movl %ecx, %cr0

	//Jump to the higher-half with and absolute jump
	lea 4f, %ecx
	jmp *%ecx

.section .text
4:
	//Paging is fully setup and enabled
	/*//Unmap the identity mapping as it is is no-longer necessary
	movl $0, boot_page_directory + 0
	movl $0, boot_page_directory + 4
	movl $0, boot_page_directory + 8
	movl $0, boot_page_directory + 12
	movl %cr3, %ecx
	movl %ecx, %cr3*/

	/* Setup the stack and clear ebp so debuggers don't trace further back from here */
	mov $stack_top, %esp
	xor %ebp, %ebp
	push %ebp
	mov %esp, %ebp

	/* Check if we were loaded by grub */
	cmp $0x2BADB002, %eax
	jne .not_multiboot

	/* Save the arguments from GRUB */
	push %eax
	push %ebx

	/* Setup what C(++) has no idea of */
	.extern early_boot_setup
	call early_boot_setup

	/* Call the global constructors. */
	.extern _init
	call _init

	/* Transfer control to the main kernel. */
	.extern kernel_main
	call kernel_main

	/* Call destructors (shouldn't reach here) */
	sub $4, %esp
	movw $0, (%esp)
	.extern __cxa_finalize
	call __cxa_finalize
	add $4, %esp

	/* Hang if kernel_main unexpectedly returns. */
	cli
.hang:
	hlt
	jmp .hang

	/* Come here if we aren't booted by a multiboot-compliant bootloader.
	Prints and error on the serial port.
	If you booted this without a multiboot-compliant bootloader and you don't
		give me serial port I am ***not*** going to help you more in this file. */
.not_multiboot:
	/* Try to assume nothing */
	/* We already setup the stack */
	cli /* Stop interrupts */
	cld /* And reset the direction flag (the serial functions are C++) */

	/* Setup serial for logging */
	.extern init_serial
	call init_serial

	/* Send the error over serial port */
	push .not_multiboot_err
	.extern writestr_serial
	call writestr_serial

	/* Hang cause what else can we do? */
	jmp .hang

.end:

.section .rodata
	.not_multiboot_err: .asciz "Feline OS CRIT: Not booted by a multiboot compliant bootloader, unable to continue."

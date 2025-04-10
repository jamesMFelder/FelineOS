/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2023 James McNaughton Felder */

#include <kernel/vtopmem.h>
#include <kernel/arch/arm/modes.inc>

.section .bss, "aw"
	/* Reserve stacks all modes */
	.align 16
	fiq_stack_bottom:
		.skip 1024 /* 1 KiB */
	fiq_stack_top:
	irq_stack_bottom:
		.skip 1024 /* 1 KiB */
	irq_stack_top:
	svc_stack_bottom:
		.skip 1024 /* 1 KiB */
	svc_stack_top:
	abt_stack_bottom:
		.skip 1024 /* 1 KiB */
	abt_stack_top:
	und_stack_bottom:
		.skip 1024 /* 1 KiB */
	und_stack_top:
	sys_stack_bottom:
		.skip 16384 /* 16 KiB */
	sys_stack_top:

.section .data
	/* Reserve space for paging tables */
	.align 14 /* 16KiB (change if enabling ttbr1) */
	translation_table_0:
		.skip 0x20000 /* 4GiB/(1MiB*4B) = address range/(first level descriptor range*descriptor size) */

.macro reset mode
cps #\mode\()_mode
ldr sp, =\mode\()_stack_top
eor fp, fp
.endm

/* The kernel entry point. */
.section .startup.text, "ax"
.global _start
_start:
	/* Setup the stack and clear the frame pointer so debuggers don't trace further back from here for all stacks */
	reset abt
	reset und
	reset sys
	reset irq
	reset fiq
	reset svc
	cps #sys_mode

	/* Map from phys_kernel_start to phys_kernel_end */
	ldr r7, =_phys_kernel_start
	/* Set options */
	bic r7, #0b10000000000000000000 // should be zero
	bic r7,  #0b1000000000000000000 // regular section, not supersection
	bic r7,   #0b100000000000000000 // (backwards-compat) global (we are the OS)
	bic r7,    #0b10000000000000000 // (backwards-compat) not shared (TODO: change if needed for multiprocessor bringup)
	bic r7,     #0b1000000000000000 // allow full access/maintain backwards compatibility
	bic r7,      #0b111000000000000 // TEX bits (combine with caching and bufferable for memory type)
	orr r7,         #0b110000000000 // full access
	bic r7,           #0b1000000000 // implementation-defined (ARM1176JZF-S: no ECC)
	bic r7,            #0b111100000 // domain
	bic r7,                #0b10000 // allow execution
	orr r7,                 #0b1000 // cached (write-through)
	bic r7,                  #0b100 // not buffered
	orr r7,                   #0b10 // section (use this, not a further page table)

	/* Setup a loop to map everything */
	ldr r5, =_phys_kernel_start
	ldr r4, =_phys_kernel_end
	/* translation_table_0 should already be aligned */
	ldr r8, =translation_table_0 - VA_OFFSET
	/* Map from VA_OFFSET */
	add r8, #VA_OFFSET / 0x40000

	.fld_loop:
	/* Store the section */
	str r7, [r8]
	/* Go to the next MiB without touching any of the status bits */
	add r7, #0x100000
	/* The section will be stored 4 bytes (1 word) later */
	add r8, #4

	/* Do this until we've covered the entire kernel */
	add r5, #0x100000
	cmp r5, r4
	ble .fld_loop

	/* And identity map to here */
	ldr r8, =translation_table_0 - VA_OFFSET
	/* Clear the address bits, but not the options */
	ldr r3, =0xfffff
	and r7, r3
	/* Setup the start and end */
	mov  r5, #0
	ldr r4, =paged_start - VA_OFFSET

	.idm_loop:
	str r7, [r8]
	add r7, #0x100000
	add r8, #4

	add r5, #0x100000
	cmp r5, r4
	ble .idm_loop

	ldr r8, =translation_table_0 - VA_OFFSET
	// ttbr0
	bic r8, #0b11111
	//         ┃┃┃┃┗ inner-noncacheable
	//         ┃┃┃┗━ non-shared
	//         ┃┃┗━━ no ECC
	//         ┗┻━━━ outer-noncacheable
	mcr p15, 0, r8, c2, c0, 0
	// ttbr1
	mov r8, #0
	mcr p15, 0, r8, c2, c0, 1
	// ttbc (translation table base control)
	mov r8, #0 // always use ttbr0
	mcr p15, 0, r8, c2, c0, 2

	/* Enable access to domain 0 as a "client" (use access permissions) */
	mov   r8, #1
	mcr   p15, 0, r8, c3, c0, 0

	mrc p15, 0, r8, c1, c0, 0
	orr r8, #0x1 // enable MMU
	orr r8, #0x800000 // disable subpages
	mcr p15, 0, r8, c1, c0, 0
	bl paged_start

.section .text
.global paged_start
paged_start:
	/* Clear the .bss section */
	/* Taken from https://wiki.osdev.org/Raspberry_Pi_Bare_Bones */
	/* Load the start and end */
	.extern bss_start
	.extern bss_end
	ldr r4, =bss_start
	ldr r9, =bss_end

	/* Load a bunch of zeros to write at once */
	mov r5, #0
	mov r6, #0
	mov r7, #0
	mov r8, #0
	b 2f

1:
	/* Store 4 at once, increasing the base register (r4) */
	stmia r4!, {r5-r8}

2:
	/* If we are still below bss_end, loop */
	cmp r4, r9
	blo 1b

	/* Setup what C(++) has no idea of */
	mov r0, r2
	.extern early_boot_setup
	bl early_boot_setup

	/* Call the global constructors. */
	.extern _init
	bl _init
	.extern init
	bl init

	/* Transfer control to the main kernel. */
	.extern kernel_main
	bl kernel_main

	/* Call destructors (shouldn't reach here) */
	.extern _fini
	bl _fini
	.extern fini
	bl fini
	mov r0, #0
	.extern __cxa_finalize
	bl __cxa_finalize

	/* Hang if kernel_main unexpectedly returns. */
.hang:
	wfi
	bl .hang

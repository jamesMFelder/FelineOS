; SPDX-License-Identifier: MIT
; Copyright (c) 2021 James McNaughton Felder
; Declare constants for the multiboot header.
MBALIGN    equ 1<<0             ; align loaded modules on page boundaries
MEMINFO  equ 1<<1             ; provide memory map
FLAGS    equ MBALIGN | MEMINFO  ; this is the Multiboot 'flag' field
MAGIC    equ 0x1BADB002       ; 'magic number' lets bootloader find the header
CHECKSUM equ -(MAGIC + FLAGS) ; checksum of above, to prove we are multiboot

; Declare a header as in the Multiboot Standard.
section .multiboot
	align 4
	dd MAGIC
	dd FLAGS
	dd CHECKSUM

; Reserve a stack for the initial thread.
section .bss
	align 16
	stack_bottom:
		resb 16384 ; 16 KiB
	stack_top:

; The kernel entry point.
section .text
global _start:function (_start.end - _start)
_start:
	mov esp, stack_top

	;Save the arguments from GRUB
	push eax
	push ebx

	; Call the global constructors.
	extern _init
	call _init

	; Transfer control to the main kernel.
	extern kernel_main
	call kernel_main

	;Call destructors (shouldn't reach here)
	sub esp, 4
	mov [esp], dword 0x0
	extern __cxa_finalize
	call __cxa_finalize
	add esp, 4

	; Hang if kernel_main unexpectedly returns.
	cli
.hang:
	hlt
	jmp 1b

.end:

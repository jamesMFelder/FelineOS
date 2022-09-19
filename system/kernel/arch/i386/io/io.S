/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2021 James McNaughton Felder */
.global outb
.global outw
.global outl
.global inb
.global inw
.global inl
.global io_wait

.macro entry
	push %esp
	mov %esp, %ebp
.endm

.macro exit
	mov %ebp, %esp
	pop %esp
.endm

.macro setup_out value_register
	mov 12(%ebp), \value_register
	mov 8(%ebp), %dx
.endm

//Val: 12(%ebp) : al, ax, eax
//Port: 8(%ebp) : dx
outb:
	entry
	setup_out %al
	outb %al, %dx
	exit
	ret

outw:
	entry
	setup_out %ax
	outw %ax, %dx
	exit
	ret

outl:
	entry
	setup_out %eax
	outl %eax, %dx
	exit
	ret

//Port: 8(%ebp)
//Val: eax
inb:
	entry
	mov 8(%ebp), %dx
	mov $0, %eax
	inb %dx, %al
	exit
	ret

inw:
	entry
	mov 8(%ebp), %dx
	mov $0, %eax
	inw %dx, %ax
	exit
	ret

inl:
	entry
	mov 8(%ebp), %dx
	inl %dx, %eax
	exit
	ret

io_wait:
	entry
	push $0x80
	call outb
	pop %eax
	exit
	ret

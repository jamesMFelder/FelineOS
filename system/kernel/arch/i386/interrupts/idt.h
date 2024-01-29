/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2023 James McNaughton Felder */
#ifndef _KERN_IDT_H
#define _KERN_IDT_H 1

#include <cstdint>

#define IDT_MAX_DESCRIPTORS 32

/* Disable interrupts */
/* Save current instruction */
#define IDT_INTERRUPT_GATE 0x8E

/* Leave interrupts on */
/* Save next instruction */
#define IDT_TRAP_GATE 0x8F

typedef struct {
		uint16_t isr_low;   /* The lower 16 bits of the ISR's address */
		uint16_t kernel_cs; /* The GDT segment selector that the CPU will load
		                       into CS before calling the ISR */
		uint8_t reserved;   /* Set to zero */
		uint8_t attributes; /* Type and attributes; see the IDT page */
		uint16_t isr_high;  /* The higher 16 bits of the ISR's address */
} __attribute__((packed)) idt_entry_t;

__attribute__((aligned(0x10))) static idt_entry_t idt[256] __attribute__((
	used)); /* Create an array of IDT entries; aligned for performance */

typedef struct {
		uint16_t limit;
		uint32_t base;
} __attribute__((packed)) idtr_t;

static idtr_t idtr
	__attribute__((used)); /* For use with the lidt instruction */

extern void *isr_stub_table[];

#endif /* _KERN_IDT_H */

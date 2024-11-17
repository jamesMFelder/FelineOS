/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2023 James McNaughton Felder */
#ifndef _KERN_IDT_H
#define _KERN_IDT_H 1

#include <cstdint>

#define IDT_MAX_DESCRIPTORS 33

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

// From https://wiki.osdev.org/PIC
#define PIC1 0x20    /* IO base address for master PIC */
#define PIC2 0xA0    /* IO base address for slave PIC */
#define PIC_EOI 0x20 /* End of interrupt command code */
#define PIC1_COMMAND PIC1
#define PIC1_DATA (PIC1 + 1)
#define PIC2_COMMAND PIC2
#define PIC2_DATA (PIC2 + 1)
void PIC_remap(int offset1, int offset2);
void IRQ_add_mask(unsigned char IRQline);
void IRQ_clear_mask(unsigned char IRQline);

#endif /* _KERN_IDT_H */

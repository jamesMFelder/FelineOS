/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2023 James McNaughton Felder */
#include <cinttypes>
#include <kernel/arch/i386/idt.h>
#include <kernel/interrupts.h>
#include <kernel/io.h>
#include <kernel/log.h>

/* This is a basic stub to be called by any Interrupt Service Routine */
void idt_set_descriptor(uint8_t vector, void *isr, uint8_t flags) {
	idt_entry_t *descriptor = &idt[vector];

	descriptor->isr_low = reinterpret_cast<uintptr_t>(isr) & 0xFFFF;
	descriptor->kernel_cs = 0x08; /* this value can be whatever offset your
	                                 kernel code selector is in your GDT */
	descriptor->attributes = flags;
	descriptor->isr_high = reinterpret_cast<uintptr_t>(isr) >> 16;
	descriptor->reserved = 0;
}

void idt_init() {
	idtr.base = reinterpret_cast<uintptr_t>(&idt[0]);
	idtr.limit = sizeof(idt_entry_t) * IDT_MAX_DESCRIPTORS - 1;

	/* Fill up the table with mostly interrupts */
	for (uint8_t vector = 0; vector < 31; vector++) {
		idt_set_descriptor(vector, isr_stub_table[vector], IDT_INTERRUPT_GATE);
	}
	/* And one trap for our syscall */
	idt_set_descriptor(31, isr_stub_table[31], IDT_TRAP_GATE);
	/* And an interrupt for the PIT */
	idt_set_descriptor(32, isr_stub_table[32], IDT_INTERRUPT_GATE);

	__asm__ volatile("mov $0xff, %al");             /* Disable the pic */
	__asm__ volatile("out %al, $0xa1");             /* Disable the pic */
	__asm__ volatile("out %al, $0x21");             /* Disable the pic */
	__asm__ volatile("lidt %0" : : "memory"(idtr)); /* load the new IDT */
	__asm__ volatile("sti");                        /* set the interrupt flag */

	/* Map the timer interrupt to 32 */
	PIC_remap(32, 40);
}

// From https://wiki.osdev.org/PIC
/* reinitialize the PIC controllers, giving them specified vector offsets
   rather than 8h and 70h, as configured by default */

#define ICW1_ICW4 0x01      /* Indicates that ICW4 will be present */
#define ICW1_SINGLE 0x02    /* Single (cascade) mode */
#define ICW1_INTERVAL4 0x04 /* Call address interval 4 (8) */
#define ICW1_LEVEL 0x08     /* Level triggered (edge) mode */
#define ICW1_INIT 0x10      /* Initialization - required! */

#define ICW4_8086 0x01       /* 8086/88 (MCS-80/85) mode */
#define ICW4_AUTO 0x02       /* Auto (normal) EOI */
#define ICW4_BUF_SLAVE 0x08  /* Buffered mode/slave */
#define ICW4_BUF_MASTER 0x0C /* Buffered mode/master */
#define ICW4_SFNM 0x10       /* Special fully nested (not) */

/*
arguments:
    offset1 - vector offset for master PIC
        vectors on the master become offset1..offset1+7
    offset2 - same for slave PIC: offset2..offset2+7
*/
void PIC_remap(int offset1, int offset2) {
	uint8_t a1, a2;

	asm("cli");
	a1 = inb(PIC1_DATA); // save masks
	a2 = inb(PIC2_DATA);

	outb(PIC1_COMMAND,
	     ICW1_INIT |
	         ICW1_ICW4); // starts the initialization sequence (in cascade mode)
	outb(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);
	outb(PIC1_DATA, offset1); // ICW2: Master PIC vector offset
	outb(PIC2_DATA, offset2); // ICW2: Slave PIC vector offset
	outb(PIC1_DATA, 4); // ICW3: tell Master PIC that there is a slave PIC at
	                    // IRQ2 (0000 0100)
	outb(PIC2_DATA, 2); // ICW3: tell Slave PIC its cascade identity (0000 0010)

	outb(PIC1_DATA,
	     ICW4_8086); // ICW4: have the PICs use 8086 mode (and not 8080 mode)
	outb(PIC2_DATA, ICW4_8086);

	outb(PIC1_DATA, a1); // restore saved masks.
	outb(PIC2_DATA, a2);

	asm("sti");
}

void IRQ_add_mask(unsigned char IRQline) {
	uint16_t port;
	uint8_t value;

	if (IRQline < 8) {
		port = PIC1_DATA;
	} else {
		port = PIC2_DATA;
		IRQline -= 8;
	}
	value = inb(port) | (1 << IRQline);
	outb(port, value);
}

void IRQ_clear_mask(unsigned char IRQline) {
	uint16_t port;
	uint8_t value;

	if (IRQline < 8) {
		port = PIC1_DATA;
	} else {
		port = PIC2_DATA;
		IRQline -= 8;
	}
	value = inb(port) & ~(1 << IRQline);
	io_wait();
	outb(port, value);
}

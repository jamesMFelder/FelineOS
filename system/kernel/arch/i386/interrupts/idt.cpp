// SPDX-License-Identifier: MIT
// Copyright (c) 2021 James McNaughton Felder
#include "idt.h"
#include <cinttypes>
#include <cstdlib>
#include <kernel/interrupts.h>
#include <kernel/log.h>

//This is a basic stub to be called by any Interrupt Service Routine
void idt_set_descriptor(uint8_t vector, void* isr, uint8_t flags) {
	idt_entry_t* descriptor = &idt[vector];

	descriptor->isr_low        = reinterpret_cast<uintptr_t>(isr) & 0xFFFF;
	descriptor->kernel_cs      = 0x08; // this value can be whatever offset your kernel code selector is in your GDT
	descriptor->attributes     = flags;
	descriptor->isr_high       = reinterpret_cast<uintptr_t>(isr) >> 16;
	descriptor->reserved       = 0;
}

void idt_init() {
	idtr.base = reinterpret_cast<uintptr_t>(&idt[0]);
	idtr.limit = sizeof(idt_entry_t) * IDT_MAX_DESCRIPTORS - 1;

	//Fill up the table with mostly interrupts
	for (uint8_t vector = 0; vector < 31; vector++) {
		idt_set_descriptor(vector, isr_stub_table[vector], IDT_INTERRUPT_GATE);
	}
	//And one trap for our syscall
	idt_set_descriptor(31, isr_stub_table[31], IDT_TRAP_GATE);

	__asm__ volatile ("mov $0xff, %al"); //Disable the pic
	__asm__ volatile ("out %al, $0xa1"); //Disable the pic
	__asm__ volatile ("out %al, $0x21"); //Disable the pic
	__asm__ volatile ("lidt %0" : : "memory"(idtr)); // load the new IDT
	__asm__ volatile ("sti"); // set the interrupt flag
}

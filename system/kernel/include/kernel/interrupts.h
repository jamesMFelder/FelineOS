/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2023 James McNaughton Felder */
#ifndef _KERN_INTERRUPTS_H
#define _KERN_INTERRUPTS_H 1

#include <cinttypes>
#include <kernel/asm_compat.h>

void idt_set_descriptor(uint8_t vector, void *isr, uint8_t flags);

/* Setup the IDT */
ASM void idt_init();
ASM void setup_irqs();
void systimer_irq_handler();

#endif /* _KERN_INTERRUPTS_H */

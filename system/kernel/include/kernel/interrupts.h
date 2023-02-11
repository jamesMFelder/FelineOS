/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2023 James McNaughton Felder */
#ifndef _KERN_INTERRUPTS_H
#define _KERN_INTERRUPTS_H 1

#include <stdint.h>

void idt_set_descriptor(uint8_t vector, void* isr, uint8_t flags);

/* Setup the IDT */
void idt_init(void);

#endif /* _KERN_INTERRUPTS_H */

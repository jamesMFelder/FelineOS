// SPDX-License-Identifier: MIT
// Copyright (c) 2021 James McNaughton Felder
#ifndef _KERN_INTERRUPTS_H
#define _KERN_INTERRUPTS_H 1

#include <stdint.h>

#ifdef __cplusplus
extern "C"{
#endif
void idt_set_descriptor(uint8_t vector, void* isr, uint8_t flags);

//Setup the IDT
void idt_init(void);
#ifdef __cplusplus
}
#endif

#endif //_KERN_INTERRUPTS_H

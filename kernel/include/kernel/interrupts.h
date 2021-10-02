// SPDX-License-Identifier: MIT
// Copyright (c) 2021 James McNaughton Felder
#ifndef _KERN_INTERRUPTS_H
#define _KERN_INTERRUPTS_H

#include <stdint.h>
#include <stdbool.h>

#include <kernel/log.h>

#ifdef __cplusplus
extern "C"{
#endif
void exception_handler(unsigned int excep_num, unsigned int error);//A stub that prints the error
void idt_set_descriptor(uint8_t vector, void* isr, uint8_t flags);

//Setup the IDT
void idt_init(void);

int add_isr(unsigned int num, void *func);
#ifdef __cplusplus
}
#endif

#endif //_KERN_INTERRUPTS_H

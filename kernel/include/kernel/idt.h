#ifndef _KERN_IDT_H
#define _KERN_IDT_H

#include <stdint.h>
#include <stdbool.h>

#include <kernel/log.h>

//TODO: get as much stuff out of here as possible now that it is public
#define IDT_MAX_DESCRIPTORS 32

typedef struct {
	uint16_t    isr_low;      // The lower 16 bits of the ISR's address
	uint16_t    kernel_cs;    // The GDT segment selector that the CPU will load into CS before calling the ISR
	uint8_t     reserved;     // Set to zero
	uint8_t     attributes;   // Type and attributes; see the IDT page
	uint16_t    isr_high;     // The higher 16 bits of the ISR's address
} __attribute__((packed)) idt_entry_t;

__attribute__((aligned(0x10)))
static idt_entry_t idt[256] __attribute__((used)); // Create an array of IDT entries; aligned for performance

typedef struct {
	uint16_t	limit;
	uint32_t	base;
} __attribute__((packed)) idtr_t;

static idtr_t idtr __attribute__((used)); //For use with the lidt instruction

void exception_handler(unsigned int excep_num, unsigned int error);//A stub that prints the error
void idt_set_descriptor(uint8_t vector, void* isr, uint8_t flags);

extern void* isr_stub_table[];

//Setup the IDT
void idt_init(void);

int add_isr(unsigned int num, void *func);

#endif //_KERN_IDT_H

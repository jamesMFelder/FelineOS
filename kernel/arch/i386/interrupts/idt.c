#include <kernel/idt.h>

//This is a basic stub to be called by any Interrupt Service Routine
__attribute__((noreturn))
void exception_handler(unsigned int excep_num, unsigned int error) {
	//klogf("Interrupt %X called.", excep_num);
	klogf("Interrupt 0x%X called with error 0x%X.", excep_num, error);
	__asm__ volatile ("hlt"); // Completely hangs the computer
	__builtin_unreachable();
	//__asm__ volatile ("sti"); //Reenable interrupts
}

void idt_set_descriptor(uint8_t vector, void* isr, uint8_t flags) {
	idt_entry_t* descriptor = &idt[vector];

	descriptor->isr_low        = (uint32_t)isr & 0xFFFF;
	descriptor->kernel_cs      = 0x08; // this value can be whatever offset your kernel code selector is in your GDT
	descriptor->attributes     = flags;
	descriptor->isr_high       = (uint32_t)isr >> 16;
	descriptor->reserved       = 0;
}

void idt_init() {
	idtr.base = (uintptr_t)&idt[0];
	idtr.limit = (uint16_t)sizeof(idt_entry_t) * IDT_MAX_DESCRIPTORS - 1;

	for (uint8_t vector = 0; vector < 32; vector++) {
		idt_set_descriptor(vector, isr_stub_table[vector], 0x8E);
		//vectors[vector] = true; //This is probably important (copied from wiki.osdev.org), but I can't figure out what it means
	}

	__asm__ volatile ("mov $0xff, %al"); //Disable the pic
	__asm__ volatile ("out %al, $0xa1"); //Disable the pic
	__asm__ volatile ("out %al, $0x21"); //Disable the pic
	__asm__ volatile ("lidt %0" : : "memory"(idtr)); // load the new IDT
	__asm__ volatile ("sti"); // set the interrupt flag
}

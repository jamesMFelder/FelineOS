#include <kernel/arch.h>

#include "gdt/gdt.h"
#include "interrupts/idt.h"

int boot_setup(){
	//Set up the terminal for logging
	terminal_initialize();

	//Setup the Global Descriptor Table to do nothing
	disable_gdt();
	
	//Enable interrupts
	idt_init();
	return 0;
}

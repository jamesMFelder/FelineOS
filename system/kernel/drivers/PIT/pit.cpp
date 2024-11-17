#include <drivers/timer.h>
#include <feline/logger.h>
#include <feline/settings.h>
#include <kernel/arch/i386/idt.h>
#include <kernel/io.h>
#include <kernel/vtopmem.h>

// From https://wiki.osdev.org/PIT
#define BASE_FREQ 1193182

// Make each tick be one millisecond (frequency 1KHz)
#define TICKS_PER_SECOND 1'000

#define RELOAD_VALUE BASE_FREQ / TICKS_PER_SECOND

void reload_pit() {
	outb(0x40, RELOAD_VALUE & 0xFF);
	io_wait();
	outb(0x40, (RELOAD_VALUE & 0xFF00) >> 8);
}

void init_timers() {
	Settings::Time::ms_since_boot.initialize(0);
	// Enable channel 0, both bytes readable, interrupt on terminal count,
	// binary mode
	outb(0x43, 0b00'11'011'0);
	reload_pit();
	IRQ_clear_mask(0);
}

ASM void PIT_isr_handler() {
	static unsigned num_ticks = 0;
	++num_ticks;
	++Settings::Time::ms_since_boot.get();

	if (num_ticks % 1'000 == 0) {
		kDbg() << dec(num_ticks / 1'000) << " seconds since boot(ish)";
	}

	// Acknowledge the interrupt
	outb(PIC1_COMMAND, PIC_EOI);
}

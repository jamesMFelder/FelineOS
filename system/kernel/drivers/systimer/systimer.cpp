#include <cstdint>
#include <drivers/timer.h>
#include <feline/settings.h>
#include <kernel/interrupts.h>
#include <kernel/io.h>
#include <kernel/log.h>
#include <kernel/mem.h>
#include <kernel/vtopmem.h>
#include <stdint.h>

typedef uint32_t volatile timer_reg;

struct SystemTimer {
		timer_reg control_status;
		timer_reg counter_low;
		timer_reg counter_high;
		timer_reg compare0;
		timer_reg compare1;
		timer_reg compare2;
		timer_reg compare3;
};

static SystemTimer *timer;

void reload_timer() {
	/* Create an interrupt each microsecond (frequency 1MHz) */
	timer->compare1 = timer->counter_low + 1'000'000;
}

void init_timers() {
	Settings::Time::ms_since_boot.initialize(0);
	/* Map the timer */
	PhysAddr<SystemTimer> timer_addr(0x2000'3000);
	auto result = map_range(timer_addr, sizeof(SystemTimer),
	                        reinterpret_cast<void **>(&timer), MAP_DEVICE);
	if (result != map_success) {
		kCritical() << "Unable to initialize timers!";
		std::abort();
	}
	/* Enable the IRQ */
	PhysAddr<uint32_t> ENABLE_IRQS_1(0x2000B210);
	write_pmem<uint32_t>(ENABLE_IRQS_1, 0x2);
	/* Start the timer */
	reload_timer();
	setup_irqs();
}

void systimer_irq_handler() {
	static uint64_t seconds_since_boot;
	Settings::Time::ms_since_boot.set(
		(static_cast<uint64_t>(timer->counter_high) << 32) |
		timer->counter_low);
	if (Settings::Time::ms_since_boot.get() / 1'000'000 > seconds_since_boot) {
		seconds_since_boot = Settings::Time::ms_since_boot.get() / 1'000'000;
		kDbg() << dec(seconds_since_boot) << " seconds since boot(ish)";
	}
	reload_timer();
}

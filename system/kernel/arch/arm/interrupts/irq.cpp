#include <cstdint>
#include <kernel/asm_compat.h>
#include <kernel/interrupts.h>
#include <kernel/paging.h>
#include <kernel/phys_addr.h>
#include <stdint.h>

uint32_t *IRQ_basic_pending;

ASM void setup_irqs() {
	PhysAddr<uint32_t> basic_pending_addr(0x2000'B000);
	map_range(PhysAddr<uint32_t>(0x2000'B200), sizeof(uint32_t),
	          reinterpret_cast<void **>(&IRQ_basic_pending), MAP_DEVICE);
}

ASM void interrupt_handler() {
	// Check interrupt status bits, most to least important.
	// Change priority by reordering the checks, or by making one a FIQ.
	uint32_t IRQ_status = *IRQ_basic_pending;
	if (IRQ_status & (1 << 8)) {
		return systimer_irq_handler();
	}
	kCriticalNoAlloc() << "Unknown interrupt " << hex(*IRQ_basic_pending);
	return;
}

#include <feline/spinlock.h>

// TODO: find a better home for this
// https://stackoverflow.com/a/54920142
#ifdef _MSC_VER
#include <intrin.h>
#define NOP() __nop()       // _emit 0x90
#else
// assume __GNUC__ inline asm
#define NOP() asm("nop")    // implicitly volatile
#endif

// This should be in the kernel, not libFeline. However, it is much simpler to have in this class
#ifndef LIBFELINE_ONLY
#ifdef __i386__
inline uint32_t Spinlock::disable_interrupts() {
	uint32_t flags;
	asm volatile ("pushf; cli; pop %0" : "=r"(flags) : : "memory");
	return flags;
}
inline void Spinlock::restore_interrupts(uint32_t flags) {
	asm("push %0; popf;" : : "rm"(flags) : "memory", "cc");
	return;
}
#elifdef __arm__
inline uint32_t Spinlock::disable_interrupts() {
	uint32_t flags;
	asm volatile ("mrs %0, cpsr; cpsid if; and %0, %0, #0xc0" : "=r"(flags) : : "memory");
	return flags;
}
inline void Spinlock::restore_interrupts(uint32_t flags) {
	uint32_t tmp;
	asm("mrs r0, cpsr; bic r0, r0, %0; msr cpsr, %0" : : "r"(flags) : "r0", "memory", "cc");
}
#else
#error "Unsupported architecture, cannot disable interupts!"
#endif
#else
inline uint32_t Spinlock::disable_interrupts() {return 0;}
inline void Spinlock::restore_interrupts(uint32_t flags) {}
#endif

/* Wait to get the lock */
void Spinlock::acquire_lock() {
	uint32_t flags = disable_interrupts(); /* Disable interrupts */
	/* If the lock is true (not held), loop */
	/* Once it is false, atomically replace with true and continue */
	while (!lock.test_and_set(std::memory_order_acquire)) {
		/* If it wasn't 0, relax the CPU so hyper-threading is more efficient */
		NOP();
	}
	stored_flags = flags; /* Save the previous interrupt state for later */
	/* Once it was 0 (released by someone else) */
	/*	We already set it to 1 */
	return;
}

bool Spinlock::try_acquire_lock() {
	uint32_t flags = disable_interrupts();
	bool result = lock.test_and_set(std::memory_order_acquire);
	if (result) {
		stored_flags = flags; /* Save the previous interrupt state for later, but don't clobber the current holder's state */
	}
	else {
		restore_interrupts(flags); /* Don't leave interrupts disabled if we don't have the lock. */
	}
	return result;
}

void Spinlock::release_lock() {
	/* Release the lock */
	uint32_t flags = stored_flags; /* Avoid race conditions when we release the lock. */
	lock.clear(std::memory_order_release);
	restore_interrupts(flags); /* Re-enable interrupts if they were enabled before. */
	return;
}

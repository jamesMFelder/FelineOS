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

/* Wait to get the lock */
void Spinlock::acquire_lock() {
	bool temp = false;
	/* If the lock is true (not held), loop */
	/* Once it is false, atomically replace with true and continue */
	while (!lock.compare_exchange_weak(temp, true, std::memory_order_seq_cst)) {
		temp = false;
		/* If it wasn't 0, relax the CPU so hyper-threading is more efficient */
		NOP();
	}
	/* Once it was 0 (released by someone else) */
	/*	We already set it to 1 */
	return;
}

bool Spinlock::try_acquire_lock() {
	bool temp = false;
	return lock.compare_exchange_weak(temp, true, std::memory_order_seq_cst);
}

void Spinlock::release_lock() {
	/* Release the lock */
	lock.store(false);
	return;
}

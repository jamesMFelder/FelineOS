#include <feline/spinlock.h>

//Initialize the lock to unlocked
Spinlock::Spinlock(){}

//Wait to get the lock
void Spinlock::aquire_lock(){
	bool temp=false;
	//If the lock is true (not held), loop
	//Once it is false, atomically replace with true and continue
	while(!lock.compare_exchange_weak(temp, true, std::memory_order_seq_cst)){
		temp=false;
		//If it wasn't 0, relax the CPU so hyperthreading is more efficient
		asm("pause");
	}
	//Once it was 0 (released by someone else)
	//	We already set it to 1
	return;
}

void Spinlock::release_lock() {
	//Release the lock
	lock.store(false);
	return;
}
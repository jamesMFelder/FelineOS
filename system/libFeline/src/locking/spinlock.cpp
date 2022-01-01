#include <feline/spinlock.h>

//Initialize the lock to unlocked
Spinlock::Spinlock(){}

//Wait to get the lock
void Spinlock::aquire_lock(){
	//Set the lock to 1 and test if it was 0
	while(!atomic_flag_test_and_set(&lock)){
		//If it wasn't 0, relax the CPU so hyperthreading is more efficient
		asm("pause");
	}
	//Once it was 0 (released by someone else)
	//	We already set it to 1
	return;
}

void Spinlock::release_lock() {
	//Release the lock
	atomic_flag_clear(&lock);
	return;
}

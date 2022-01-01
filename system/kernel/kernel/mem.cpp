#include <kernel/mem.h>

page::page(void const * const virt_addr){
	//Do the cast
	addr=reinterpret_cast<uintptr_t>(virt_addr);
	//And cut of the lower bytes
	addr&=~(4_KiB-1);
}

page::page(uintptr_t const virt_addr){
	addr=virt_addr;
	addr&=~(4_KiB-1);
}

//Prefix increment
page& page::operator++(){
	//Make sure we won't overflow
	if(addr<=UINTPTR_MAX-4_KiB){
		addr+=4_KiB;
	}
	else{
		addr=UINTPTR_MAX;
	}
	return *this;
}

//Postfix increment
page page::operator++(int){
	//Create a copy
	page temp=*this;
	//Do the increment
	operator++();
	//Return the copy
	return temp;
}

//Prefix decrement
page& page::operator--(){
	//Make sure we won't underflow
	if(addr>=4_KiB){
		addr+=4_KiB;
	}
	//If we would: set to 0
	else{
		addr=0;
	}
	return *this;
}

//Postfix decrement
page page::operator--(int){
	//Create a copy
	page temp=*this;
	//Do the increment
	operator--();
	//Return the copy
	return temp;
}

//Return the address
void *page::get() const{
	return reinterpret_cast<void*>(addr);
}

//Return the address as a uintptr_t (not encouraged)
uintptr_t page::getInt() const{
	return addr;
}

//Set the address (rounding off)
void page::set(const void *newAddr){
	addr=reinterpret_cast<uintptr_t>(newAddr);
	addr&=~(4_KiB-1);
}

//Implicit get (encourage only taking pointer)
page::operator void*() const{
	return reinterpret_cast<void*>(addr);
}

//Check if it is null
bool page::isNull() const{
	return addr==0;
}

#include <cstdlib>
#include <kernel/mem.h>

page::page(void const * const virt_addr){
	/* Do the cast */
	addr=reinterpret_cast<uintptr_t>(virt_addr);
	/* And cut of the lower bytes */
	addr&=~(get_chunk_size()-1);
}

page::page(uintptr_t const virt_addr){
	addr=virt_addr;
	addr&=~(get_chunk_size()-1);
}

largePage::largePage(void const * const virt_addr)
	:page(virt_addr){}

largePage::largePage(uintptr_t const virt_addr)
	:page(virt_addr){}

/* Prefix increment */
page& page::operator++(){
	/* Make sure we won't overflow */
	if(addr<=UINTPTR_MAX-get_chunk_size()){
		addr+=get_chunk_size();
	}
	else{
		addr=UINTPTR_MAX;
	}
	return *this;
}

/* Postfix increment */
page page::operator++(int){
	/* Create a copy */
	page temp=*this;
	/* Do the increment */
	operator++();
	/* Return the copy */
	return temp;
}

/* Prefix decrement */
page& page::operator--(){
	/* Make sure we won't underflow */
	if(addr>=get_chunk_size()){
		addr-=get_chunk_size();
	}
	/* If we would: set to 0 */
	else{
		addr=0;
	}
	return *this;
}

/* Postfix decrement */
page page::operator--(int){
	/* Create a copy */
	page temp=*this;
	/* Do the increment */
	operator--();
	/* Return the copy */
	return temp;
}

/* Self-addition */
page& page::operator+=(const page &rhs){
	/* Make sure we won't overflow */
	if (addr<UINTPTR_MAX-rhs.getInt()){
		addr+=rhs.getInt();
	}
	else{
		addr=UINTPTR_MAX;
		std::abort();
	}
	return *this;
}

/* Addition */
page operator+(page lhs, const page &rhs){
	lhs += rhs;
	return lhs;
}

/* Return the address */
void *page::get() const{
	return reinterpret_cast<void*>(addr);
}

/* Return the address as a uintptr_t (not encouraged) */
uintptr_t page::getInt() const{
	return addr;
}

/* Set the address (rounding off) */
void page::set(const void *newAddr){
	addr=reinterpret_cast<uintptr_t>(newAddr);
	addr&=~(get_chunk_size()-1);
}

/* Implicit get (encourage only taking pointer) */
page::operator void*() const{
	return reinterpret_cast<void*>(addr);
}

/* Check if it is null */
bool page::isNull() const{
	return addr==0;
}

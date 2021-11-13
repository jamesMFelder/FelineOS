#include <bits/io.h>

#include <terminals/vga/vga_text.h>
#include <drivers/serial.h>

#if defined(__is_libk)

//TODO: good stdio
//vga_text_term defaultTerm;

int __internal_putchar(char const c){
	put_serial(c);
	//defaultTerm.putchar(c);
	return 0;
}

int __internal_writeStr(char const * const s){
	writestr_serial(s);
	//defaultTerm.puts(s);
	return 0;
}
#else //__is_libk
#error "Userspace stdio not implimented yet."
#endif //__is_libk (else)

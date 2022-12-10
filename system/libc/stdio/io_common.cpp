#include <bits/io.h>

/* #include <terminals/vga/vga_text.h> */
#include <drivers/serial.h>

#if defined(__is_libk)

/* TODO: good stdio */
/* vga_text_term defaultTerm; */

int __internal_putchar(char const c){
	//TODO: check if in text mode first
	if (c=='\n') __internal_putchar('\r');
	put_serial(c);
	/* defaultTerm.putchar(c); */
	return 0;
}

int __internal_writeStr(char const * const s){
	//TODO: check if in text mode first, otherwise use writestr_serial
	for (auto *c=s; *c!='\0'; ++c) {
		__internal_putchar(*c);
	}
	/* defaultTerm.puts(s); */
	return 0;
}
#else /* __is_libk */
#error "Userspace stdio not implimented yet."
#endif /* __is_libk (else) */

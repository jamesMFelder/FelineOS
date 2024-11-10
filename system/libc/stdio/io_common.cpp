#include <bits/io.h>
#include <drivers/serial.h>
#include <drivers/terminal.h>

#if defined(__is_libk)

extern vga_text_term term;

int __internal_putchar(char const c) {
	// TODO: check if in text mode first
	if (c == '\n')
		__internal_putchar('\r');
	put_serial(c);
	term.putchar(c);
	return 0;
}

int __internal_writeStr(char const *const s) {
	writestr_serial(s);
	for (const char *c = s; *c != '\0'; c++) {
		term.putchar(*c);
	}
	return 0;
}
#else /* __is_libk */
#error "Userspace stdio not implimented yet."
#endif /* __is_libk (else) */

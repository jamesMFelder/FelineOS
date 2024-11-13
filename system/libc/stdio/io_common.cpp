#include <bits/io.h>
#include <drivers/serial.h>
#include <drivers/terminal.h>

#if defined(__is_libk)

#ifdef __i386__
extern vga_text_term term;

static int term_putchar(char const c) { return term.putchar(c); }
static int term_putstr(char const *s) { return term.putstr(s); }
#else  // __i386__
int term_putchar(char const) { return 0; }
int term_putstr(char const *) { return 0; }
#endif // __i386__ (else)

int __internal_putchar(char const c) {
	// TODO: check if in text mode first
	if (c == '\n')
		__internal_putchar('\r');
	put_serial(c);
	term_putchar(c);
	return 0;
}

int __internal_writeStr(char const *const s) {
	writestr_serial(s);
	term_putstr(s);
	return 0;
}
#else /* __is_libk */
#error "Userspace stdio not implimented yet."
#endif /* __is_libk (else) */

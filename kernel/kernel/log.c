#include <kernel/log.h>

//For the moment, just write to terminal.
void kerror(const char *data){
	terminal_writestring("FelineOS ERR: ");
	terminal_writestring(data);
	terminal_putchar('\n');
	return;
}
void kwarn(const char *data){
	terminal_writestring("FelineOS WARN: ");
	terminal_writestring(data);
	terminal_putchar('\n');
	return;
}
void klog(const char *data){
	terminal_writestring("FelineOS LOG: ");
	terminal_writestring(data);
	terminal_putchar('\n');
	return;
}

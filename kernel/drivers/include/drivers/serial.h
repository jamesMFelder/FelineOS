#ifndef _KERN_DRIVER_SERIAL_H
#define _KERN_DRIVER_SERIAL_H

#include <kernel/asm.h>

#define PORT 0x3f8 //COM1

int init_serial();
char read_serial();
void write_serial(char a);

#endif //_KERN_DRIVER_SERIAL_H

// SPDX-License-Identifier: MIT
// Copyright (c) 2021 James McNaughton Felder
#ifndef _KERN_DRIVER_SERIAL_H
#define _KERN_DRIVER_SERIAL_H 1

#include <kernel/asm.h>
#include <stddef.h>

#define PORT 0x3f8 //COM1

int init_serial();
char read_serial();
void put_serial(char a);
void write_serial(const char *str, const size_t len);
void writestr_serial(const char *str);

#endif //_KERN_DRIVER_SERIAL_H

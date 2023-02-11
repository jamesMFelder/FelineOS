/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2023 James McNaughton Felder */
#ifndef _KERN_DRIVER_SERIAL_H
#define _KERN_DRIVER_SERIAL_H 1

#include <stddef.h>
#include <kernel/asm_compat.h>

/* Setup the serial port (ASM because of emergency at initialization possibility) */
ASM int init_serial();
/* Read a character from the serial port */
char read_serial();
/* Write a character to the serial port */
void put_serial(char a);
/* Write len bytes from str to the serial port (ignoring null characters) */
void write_serial(const char *str, const size_t len);
/* Write from str to the next null (ASM because of emergency at initialization posibility) */
ASM void writestr_serial(const char *str);

#endif /* _KERN_DRIVER_SERIAL_H */

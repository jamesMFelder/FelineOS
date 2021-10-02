// SPDX-License-Identifier: MIT
// Copyright (c) 2021 James McNaughton Felder
#ifndef _KERNEL_TTY_H
#define _KERNEL_TTY_H

#include <stddef.h>
#include <stdint.h>

//| bg | fg |
//|....|....|
#define color_normal_dark 0xF0
#define color_normal_light 0x0F
#define color_ok_dark 0x02
#define color_ok_light 0xF2
#define color_bad_dark 0x04
#define color_bad_light 0xF4

#ifdef __cplusplus
extern "C"{
#endif
//Setup the hardware
void terminal_initialize(void);
//Write a character at the next location
void terminal_putchar(char c);
//Write size bytes from data at the next location
void terminal_write(const char* data, size_t size);
//Write data at the next location
void terminal_writestring(const char* data);
//Move the next output location
//return value has bit 1 set if x is to large
//return value has bit 2 set if y is to large
int terminal_move(size_t x, size_t y);

//Set the default output color
void terminal_setcolor(uint8_t color);
#ifdef __cplusplus
}
#endif

#endif // _KERNEL_TTY_H

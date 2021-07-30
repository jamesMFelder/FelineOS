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

void terminal_initialize(void);
void terminal_putchar(char c);
void terminal_write(const char* data, size_t size);
void terminal_writestring(const char* data);

void terminal_setcolor(uint8_t color);

#endif // _KERNEL_TTY_H

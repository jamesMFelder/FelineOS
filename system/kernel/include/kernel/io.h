/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2021 James McNaughton Felder */
#ifndef KERNEL_IO_H
#define KERNEL_IO_H

#include <cstdint>
#include <kernel/asm_compat.h>

ASM void outb(uint16_t port, uint8_t val);

ASM void outw(uint16_t port, uint16_t val);

ASM void outl(uint16_t port, uint32_t val);

ASM uint8_t inb(uint16_t port);

ASM uint16_t inw(uint16_t port);

ASM uint32_t inl(uint16_t port);

ASM void io_wait(void);

#endif // KERNEL_IO_H

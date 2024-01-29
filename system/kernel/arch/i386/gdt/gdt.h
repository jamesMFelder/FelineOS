/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2023 James McNaughton Felder */
#ifndef _KERN_GDT_H
#define _KERN_GDT_H 1

#include <cstddef>
#include <cstdint>
#include <kernel/asm_compat.h>
#include <kernel/log.h>

/* This warning doesn't matter here, unlike a lot of the kernel */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpadded"
struct GDT {
		uintptr_t base;
		size_t limit;
		uint8_t type;
};
#pragma GCC diagnostic pop

/* target is a pointer to the 8-byte GDT entry */
/* source is an arbitrary structure describing the GDT entry */
void encodeGdtEntry(uint8_t *target, struct GDT source);

/* gdt is the target struct from encodeGDTEntry */
ASM void setGdt(uint64_t *GDT, unsigned int gdt_size);

/* Creates segments spanning the entire memory for everything. */
void disable_gdt();

#endif /* _KERN_GDT_H */

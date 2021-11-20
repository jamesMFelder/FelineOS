// SPDX-License-Identifier: MIT
// Copyright (c) 2021 James McNaughton Felder
#ifndef _KERN_GDT_H
#define _KERN_GDT_H 1

#include <stdint.h>
#include <kernel/log.h>
#include <stddef.h>

struct GDT{
	uintptr_t base;
	size_t limit;
	uint8_t type;
};

// target is a pointer to the 8-byte GDT entry
// source is an arbitrary structure describing the GDT entry
void encodeGdtEntry(uint8_t *target, struct GDT source);

#ifdef __cplusplus
extern "C"{
#endif
//gdt is the target struct from encodeGDTEntry
void setGdt(uint64_t *GDT, unsigned int gdt_size);
#ifdef __cplusplus
}
#endif

//Creates segments spanning the entire memory for everything.
void disable_gdt();

#endif //_KERN_GDT_H

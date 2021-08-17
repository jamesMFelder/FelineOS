#ifndef _KERN_GDT_H
#define _KERN_GDT_H

#include <stdint.h>
#include <kernel/log.h>

struct GDT{
	int base;
	int limit;
	int type;
};

// target is a pointer to the 8-byte GDT entry
// source is an arbitrary structure describing the GDT entry
void encodeGdtEntry(uint8_t *target, struct GDT source);

//gdt is the target struct from encodeGDTEntry
void setGdt(uint64_t *GDT, unsigned int gdt_size);

//Creates segments spanning the entire memory for everything.
void disable_gdt();

#endif //_KERN_GDT_H
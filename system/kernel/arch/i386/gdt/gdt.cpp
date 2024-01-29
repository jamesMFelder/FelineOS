/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2023 James McNaughton Felder */
#include "gdt.h"
#include <kernel/vtopmem.h>

/* target is a pointer to the 8-byte GDT entry */
/* source is an arbitrary structure describing the GDT entry */
void encodeGdtEntry(uint8_t *target, struct GDT source) {
	/* Check the limit to make sure that it can be encoded */
	if ((source.limit > 65536) && ((source.limit & 0xFFF) != 0xFFF)) {
		kerror("Invalid GDT struct!");
		kerror("*target untouched!");
		return;
	}
	if (source.limit > 65536) {
		/* Adjust granularity if required */
		source.limit = source.limit >> 12;
		target[6] = 0xC0;
	} else {
		target[6] = 0x40;
	}

	/* Encode the limit */
	target[0] = source.limit & 0xFF;
	target[1] = (source.limit >> 8) & 0xFF;
	target[6] |= (source.limit >> 16) & 0xF;

	/* Encode the base */
	target[2] = source.base & 0xFF;
	target[3] = (source.base >> 8) & 0xFF;
	target[4] = (source.base >> 16) & 0xFF;
	target[7] = (source.base >> 24) & 0xFF;

	/* And... Type */
	target[5] = source.type;
}

void disable_gdt() {
	/* TODO: add another for the TSS once we create it. */
	/* These are generated from utils/gdt_create.c */
	static uint64_t gdt[] = {0x0000000000000000, 0x00CF9A000000FFFF,
	                         0x00CF92000000FFFF, 0x00CFFA000000FFFF,
	                         0x00CFF2000000FFFF};
	setGdt(gdt, sizeof(gdt));
}

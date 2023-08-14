/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2023 James McNaughton Felder */
#include "idt.h"
#include <cinttypes>
#include <cstdlib>
#include <kernel/log.h>
#include <kernel/halt.h>

ASM [[noreturn]] void handle_abort(void *executing_address, uint32_t fault_status, void *fault_address);

/* Handle an ARM data abort */
void handle_abort(void *executing_address, uint32_t fault_status, void *fault_address) {
	/* The reason is encoded in DFSR[10,3:0] (seriously) */
	auto cause = fault_status & 0b1111;
	if (fault_status & 0b1'00000'00000) {
		if (fault_status & 0b0110) {
			kcriticalf("Imprecise data abort, halting!");
		}
		else {
			kcriticalf("Reserved abort status %#" PRIb32 " with DFSR[10]=1 occured, halting!", cause);
		}
		halt();
	}
	/* Go through the causes
	 * (ordered by priority according to the technical reference manual
	 * (page 351 or 6-34), not counting ones with DFSR[10]=1) */
	switch (cause) {

		case 0b0001:
			/* Alignment Issue */
			kerrorf("Instruction %p attempted to access unaligned address %p", executing_address, fault_address);
			/* TODO: can we fix it? */
			halt();

		case 0b0000:
			/* TLB miss */
			kcritical("I'm unsure if this is supposed to be a TLB miss abort or not happening. Please fix if you encounter this. Halting now.");
			halt();

		case 0b0100:
			/* Instruction Cache Maintainance Fault (TODO: flush instruction cache?) */
			kcriticalf("Instruction cache corrupted! TODO: fix it! (note: exception occured while executing %p)", executing_address);
			halt();
			// break;

		case 0b1100:
			/* External Abort on Translation on first level */
		case 0b1110:
			/* External Abort on Translation on second level */
			kcriticalf("External abort attempting to access %p (reason = %#" PRIb32 "). Halting!", fault_address, cause);
			halt();
		case 0b0101:
			/* Translation Fault on section */
		case 0b0111:
			/* Translation Fault on page */
			kcriticalf("Translation fault attempting to access %p (reason = %#" PRIb32 "). Halting!", fault_address, cause);
			std::abort();
		case 0b0011:
			/* Access Bit Fault, Force AP Only (permission denied?) on section (TODO: what?) */
		case 0b0110:
			/* Access Bit Fault, Force AP Only (permission denied?) on page (TODO: what?) */
			kcriticalf("Permission denied attempting to access %p (reason = %#" PRIb32 "). Halting!", fault_address, cause);
			halt();
		case 0b1001:
			/* Domain Fault on section */
		case 0b1011:
			/* Domain Fault on page */
			kcriticalf("Domains not yet implimented, so abort status %#" PRIb32 " should not be possible. Halting!", cause);
			halt();
		case 0b1101:
			/* Permission Error on section */
		case 0b1111:
			/* Permission Error on page */
			kcriticalf("Permission error: abort status %#" PRIb32 " with DFSR[10]=0. Halting!", cause);
			halt();

		case 0b1000:
			/* Precise External Abort */
			kerrorf("Instruction %p aborted trying to access %p", executing_address, fault_address);
			std::abort();
			// return;

		case 0b0010:
			/* Instruction Debug Event */
			kerrorf("Debug Event: this should probably switch to the debugger, but just halting for now");
			halt();

		default:
			kcriticalf("Reserved abort status %#" PRIb32 " with DFSR[10]=0 occured, halting!", cause);
			halt();
	}
	__builtin_unreachable();
}
